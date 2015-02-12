//! @file   packetbuffer.h
#pragma once

#include "shared/linklist.h"


//! Forward declarations
class PacketHandler;


class PacketBuffer
{
    // Structure
public:
    typedef struct _PACKET_OFF_BUFFER_LIST
    {
        LIST_ENTRY ListEntry;
        ULONG Length;
        UCHAR Data[1];
    } PACKET_OFF_BUFFER_LIST, *PPACKET_OFF_BUFFER_LIST;

    // Constructer/Destructer
public:
    PacketBuffer();
    virtual ~PacketBuffer();

    // Methods
public:
    void SetHandler(PacketHandler* Handler) { handler_ = Handler; }

private:
    void Init();
    void Release();

    int GetLength(PUCHAR Buffer);
    void Parse(PUCHAR Buffer, int Length);

    void PushOffBufferList(PUCHAR Buffer, int Length);
    void ClearOffBufferList();

public:
    int Recv(char* Buffer, int Length);
    int Send(char* Buffer, int Length);

    // Data Members
private:
    ULONG ctrlFlag_;

    HANDLE offHeap_;
    LIST_ENTRY offList_;
    ULONG offListLen_;

    PacketHandler* handler_;
};
