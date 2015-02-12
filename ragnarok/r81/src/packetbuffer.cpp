//! @file   packetbuffer.cpp

#include "stdafx.h"
#include "module.h"
#include "client.h"
#include "packetbuffer.h"
#include "packethandler.h"

#define _NEW_OFF_BUFFER


//! Constructer
PacketBuffer::PacketBuffer() :
    offHeap_(NULL),
    offListLen_(0),
    handler_(NULL)
{
    Init();
}

//! Destructer
PacketBuffer::~PacketBuffer()
{
    Release();
}

//! Initialize
void PacketBuffer::Init()
{
    if (offHeap_ == NULL)
        offHeap_ = HeapCreate(0, 0xFF<<7, 0);

    if (offHeap_)
    {
        // Enable LFH
        ULONG value = 2;
        HeapSetInformation(offHeap_, HeapCompatibilityInformation, &value, sizeof(ULONG));
    }
    else
    {
        //*** Use process heap.
        offHeap_ = GetProcessHeap();
    }

    InitializeListHead(&offList_);
    offListLen_ = 0;
}

//! Release
void PacketBuffer::Release()
{
    if (offHeap_ != NULL)
    {
        if (offHeap_ != GetProcessHeap())
            HeapDestroy(offHeap_);
        else
            ClearOffBufferList();
    }

    handler_ = NULL;
}

//! Recv
int PacketBuffer::Recv(char* Buffer, int Length)
{
    DbgPrintW(L"PACKET_MEMSTATE OFF=%d CUR=%d\n", offListLen_, Length);

    PUCHAR inBuf = reinterpret_cast<PUCHAR>(Buffer);
    int inLen = Length;
    int inOfs = 0;

    if (IsListEmpty(&offList_))
    {
        if (ctrlFlag_ & CTRL_OIDCHECK)
        {
            if (Length >= 4)
            {
                DbgPrintW(L"PACKET_OIDCHECK M=%08X P=%08X\n", Client::GetOwnOID(), *reinterpret_cast<ULONG*>(Buffer));

                if (Client::IsOwnOID(reinterpret_cast<PUCHAR>(Buffer)))
                {
                    inLen -= 4;
                    inOfs += 4;
                    ctrlFlag_ ^= CTRL_OIDCHECK;
                }
            }
        }
    }
    else
    {
        if (offListLen_+Length < 2)
        {
            // 現実的に 99% 有り得ないけど、理論上は有り得るジレンマ。
            PushOffBufferList(reinterpret_cast<PUCHAR>(Buffer), Length);
            return Length;
        }
        else
        {
            SIZE_T reqLen = offListLen_ + Length;
            PUCHAR buf = reinterpret_cast<PUCHAR>(HeapAlloc(offHeap_, HEAP_ZERO_MEMORY, reqLen));

            int offset = 0;
#if defined(_NEW_OFF_BUFFER)
            for (PLIST_ENTRY entry=offList_.Flink; entry!=&offList_; entry=entry->Flink)
            {
                PPACKET_OFF_BUFFER_LIST item = CONTAINING_RECORD(entry, PACKET_OFF_BUFFER_LIST, ListEntry);
                RtlCopyMemory(buf+offset, item->Data, item->Length);
                offset += item->Length;
            }
#else
            while (IsListEmpty(&offList_)==FALSE)
            {
                PLIST_ENTRY entry = RemoveHeadList(&offList_);
                PPACKET_OFF_BUFFER_LIST item = CONTAINING_RECORD(entry, PACKET_OFF_BUFFER_LIST, ListEntry);
                RtlCopyMemory(buf+offset, item->Data, item->Length);
                offset += item->Length;
                HeapFree(offHeap_, 0, item);
            }
            offListLen_ = 0;
#endif

            RtlCopyMemory(buf+offset, Buffer, Length);

            int curLen = GetLength(buf);
            if (curLen == -1)
                curLen = *reinterpret_cast<USHORT*>(buf+2);

            if (curLen > reqLen)
            {
#if defined(_NEW_OFF_BUFFER)
                PushOffBufferList(reinterpret_cast<PUCHAR>(Buffer), Length);
#else
                PushOffBufferList(buf, reqLen);
#endif
                HeapFree(offHeap_, 0, buf);
                return Length;
            }
            else
            {
                Parse(buf, curLen);
                handler_->Recv(buf, curLen);

                HeapFree(offHeap_, 0, buf);
#if defined(_NEW_OFF_BUFFER)
                ClearOffBufferList();
#endif

                inOfs = curLen - offset;
                inLen -= inOfs;
            }
        }
    }

    while (inLen >= 2)
    {
        PUCHAR curBuf = inBuf + inOfs;

        int curLen = GetLength(curBuf);
        if (curLen == -1)
        {
            if (inLen < 4)
                break;
            else
                curLen = *reinterpret_cast<USHORT*>(curBuf+2);
        }

        if (curLen > inLen)
            break;
        else
        {
            Parse(curBuf, curLen);
            handler_->Recv(curBuf, curLen);
            inOfs += curLen;
            inLen -= curLen;
        }
    }

    if (inLen > 0)
        PushOffBufferList(inBuf, inLen);

    return Length;
}

//! Send
int PacketBuffer::Send(char* Buffer, int Length)
{
    return 0;
}

//! Parse
void PacketBuffer::Parse(PUCHAR Buffer, int Length)
{
    switch (*reinterpret_cast<USHORT*>(Buffer))
    {
        case 0x0071: ctrlFlag_ |= CTRL_OIDCHECK; break;
// このタイミングで CTRL_OIDCHECK を入れると後続が正常にパースできない可能性がある。(2008-11-19aaRagexe)
//        case 0x0227: ctrlFlag_ |= CTRL_OIDCHECK; break;
        // 0x01=Account 0x02=Char
        case 0x0259: ctrlFlag_ |= (*reinterpret_cast<UCHAR*>(Buffer+2)==0x02) ? CTRL_OIDCHECK : 0; break;
    }
}

//! -
int PacketBuffer::GetLength(PUCHAR Buffer)
{
    // client side
    return Client::GetPacketLength(Buffer);
}

//! Push new entry into OffBuffer list.
void PacketBuffer::PushOffBufferList(PUCHAR Buffer, int Length)
{
    SIZE_T reqLen = Length + sizeof(PACKET_OFF_BUFFER_LIST);

    PVOID buf = HeapAlloc(offHeap_, HEAP_ZERO_MEMORY, reqLen);
    if (buf != NULL)
    {
        PPACKET_OFF_BUFFER_LIST item = reinterpret_cast<PPACKET_OFF_BUFFER_LIST>(buf);
        item->Length = Length;
        RtlCopyMemory(item->Data, Buffer, Length);
        InsertTailList(&offList_, &item->ListEntry);

        offListLen_ += Length;
    }
}

//! Clear all entries.
void PacketBuffer::ClearOffBufferList()
{
    while (IsListEmpty(&offList_)==FALSE)
    {
        PLIST_ENTRY entry = RemoveHeadList(&offList_);
        PPACKET_OFF_BUFFER_LIST item = CONTAINING_RECORD(entry, PACKET_OFF_BUFFER_LIST, ListEntry);
        HeapFree(offHeap_, 0, item);
    }

    offListLen_ = 0;
}
