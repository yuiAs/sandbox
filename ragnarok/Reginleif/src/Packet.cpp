#include "Packet.h"
#include "Module.h"
#include "Client.h"

//------------------------------------------------------------------------------
//  Function:   Packet::Packet
//  Note    :   
//------------------------------------------------------------------------------
Packet::Packet()
    : m_Heap(NULL)
{
}

//------------------------------------------------------------------------------
//  Function:   Packet::~Packet
//  Note    :   
//------------------------------------------------------------------------------
Packet::~Packet()
{
    Release();
}

//------------------------------------------------------------------------------
//  Function:   Packet::Init
//  Note    :   
//------------------------------------------------------------------------------
void Packet::Init()
{
    DbgPrintW(_L("Packet::Init Tick=%08X\n"), GetTickCount());

    m_PacketLength = g_Ctx.Addr[VA_PACKETLENGTHCONTAINER];
    m_SkipOID = FALSE;
    m_CryptRound1 = FALSE;
    m_CryptRound2 = FALSE;
    RtlZeroMemory(m_Seed1, sizeof(m_Seed1));

    if (m_Heap == NULL)
        m_Heap = CreatePrivateHeap(0, 2048<<4, 0);

    InitializeListHead(&m_Container);
    m_ContainerLength = 0;
}

//------------------------------------------------------------------------------
//  Function:   Packet::Release
//  Note    :   
//------------------------------------------------------------------------------
void Packet::Release()
{
    if (m_Heap)
    {
        if (m_Heap != GetProcessHeap())
            HeapDestroy(m_Heap);
        else
        {
            while (IsListEmpty(&m_Container)==FALSE)
            {
                PLIST_ENTRY Entry = RemoveHeadList(&m_Container);
                PLINKCONTAINER Item = CONTAINING_RECORD(Entry, LINKCONTAINER, ListEntry);
                HeapFree(m_Heap, 0, Item);
            }
        }
    }
}

//------------------------------------------------------------------------------
//  Function:   Packet::Recv
//  Note    :   
//------------------------------------------------------------------------------
int Packet::Recv(char* Buffer, int Length)
{
    DbgPrintW(_L("Packet::Recv Tick=%08X Length=%d\n"), GetTickCount(), Length);
    DbgPrintW(_L("Packet::m_ContainerLength=%d\n"), m_ContainerLength);

    PUCHAR InBuf = reinterpret_cast<PUCHAR>(Buffer);
    int InLen = Length;
    int InOfs = 0;

    if (IsListEmpty(&m_Container))
    {
        if (m_SkipOID)
        {
            if (Length >= 4)
            {
                if (IsOwnOID(InBuf))
                {
                    InLen -= 4;
                    InOfs += 4;
                    m_SkipOID = FALSE;
                }
            }
        }
    }
    else
    {
        if ((m_ContainerLength+InLen) < 4)
        {
            AllocEntry(InBuf, InLen);
            return Length;
        }
        else
        {
            ULONG PrevLen = m_ContainerLength + InLen;
            ULONG PrevOfs = 0;
            PUCHAR PrevBuf = reinterpret_cast<PUCHAR>(VirtualAllocEx(GetCurrentProcess(), NULL, PrevLen, MEM_COMMIT, PAGE_READWRITE));

            while (IsListEmpty(&m_Container)==FALSE)
            {
                PLIST_ENTRY Entry = RemoveHeadList(&m_Container);
                PLINKCONTAINER Item = CONTAINING_RECORD(Entry, LINKCONTAINER, ListEntry);
                RtlCopyMemory(PrevBuf+PrevOfs, Item->Data, Item->Length);
                PrevOfs += Item->Length;
                HeapFree(m_Heap, 0, Item);
            }

            if (m_ContainerLength < 4)
                RtlCopyMemory(PrevBuf+PrevOfs, InBuf, 4-m_ContainerLength);

            int CurLen = GetLength(PrevBuf);
            if (CurLen == -1)
                CurLen = *reinterpret_cast<USHORT*>(PrevBuf+2);

            if (PrevLen < static_cast<ULONG>(CurLen))
            {
                m_ContainerLength = 0;
                RtlCopyMemory(PrevBuf+PrevOfs, InBuf, InLen);
                AllocEntry(PrevBuf, PrevLen);
                VirtualFreeEx(GetCurrentProcess(), PrevBuf, 0, MEM_RELEASE);
                return Length;
            }
            else
            {
                RtlCopyMemory(PrevBuf+PrevOfs, InBuf, CurLen-m_ContainerLength);
                PreRecvParse(PrevBuf, CurLen);
                VirtualFreeEx(GetCurrentProcess(), PrevBuf, 0, MEM_RELEASE);

                InOfs = CurLen - m_ContainerLength;
                InLen -= InOfs;
                m_ContainerLength = 0;
            }
        }
    }

    while (InLen >= 2)
    {
        PUCHAR CurBuf = InBuf + InOfs;

        int CurLen = GetLength(CurBuf);
        if (CurLen == -1)
        {
            if (InLen < 4)
                break;
            else
                CurLen = *reinterpret_cast<USHORT*>(CurBuf+2);
        }

        if (CurLen > InLen)
            break;
        else
        {
            PreRecvParse(CurBuf, CurLen);
            InOfs += CurLen;
            InLen -= CurLen;
        }
    }

    if (InLen > 0)
        AllocEntry(InBuf+InOfs, InLen);

    return Length;
}

//------------------------------------------------------------------------------
//  Function:   Packet::Send
//  Note    :   
//------------------------------------------------------------------------------
int Packet::Send(char* Buffer, int Length)
{
    return 0;
}

//------------------------------------------------------------------------------
//  Function:   Packet::GetLength
//  Note    :   
//------------------------------------------------------------------------------
int Packet::GetLength(PUCHAR Buffer)
{
    return ClGetPacketLength(Buffer);
}

//------------------------------------------------------------------------------
//  Function:   Packet::PreRecvParse
//  Note    :   
//------------------------------------------------------------------------------
int Packet::PreRecvParse(PUCHAR Buffer, int Length)
{
    switch (*reinterpret_cast<USHORT*>(Buffer))
    {
        case PACKET_GG_KEY16_SV:
            m_SkipOID = TRUE;
            break;
        case PACKET_CC_ACK_SELECT_CHARACTER:
            m_SkipOID = TRUE;
            break;
        // 0x0259 <SVType>.B
        // 0x01: Account
        // 0x02: Char
        case PACKET_GG_ACK_REQ_QUERY:
            m_SkipOID = (*reinterpret_cast<UCHAR*>(Buffer+2)==0x02) ? TRUE : FALSE;
            break;
    }

    return PacketHandler::RecvParse(Buffer, Length);
}

//------------------------------------------------------------------------------
//  Function:   Packet::PreSendParse
//  Note    :   
//------------------------------------------------------------------------------
int Packet::PreSendParse(PUCHAR Buffer, int Length)
{
    return 0;
}

//------------------------------------------------------------------------------
//  Function:   Packet::AllocEntry
//  Note    :   
//------------------------------------------------------------------------------
void Packet::AllocEntry(PUCHAR Buffer, int Length)
{
    SIZE_T AllocLen = Length + sizeof(LINKCONTAINER);
    PVOID AllocBuf = HeapAlloc(m_Heap, HEAP_ZERO_MEMORY, AllocLen);
    if (AllocBuf != NULL)
    {
        PLINKCONTAINER Item = reinterpret_cast<PLINKCONTAINER>(AllocBuf);
        Item->Length = Length;
        RtlCopyMemory(Item->Data, Buffer, Length);

        InsertTailList(&m_Container, &Item->ListEntry);
        m_ContainerLength += Length;
    }
}

