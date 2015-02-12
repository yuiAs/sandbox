//! @file   packet.cpp

#define INCL_WINSOCK_API_TYPEDEFS 1
#include <winsock2.h>
#include "stdafx.h"
#include "module.h"
#include "packetbuffer.h"
#include "packethandler.h"


namespace Packet
{
//! Variables
PacketBuffer* adapter = NULL;
PacketHandler* handler = NULL;
LPFN_RECV pfnRecv = NULL;
LPFN_SEND pfnSend = NULL;

//! Initialize
void Init()
{
    if (adapter == NULL)
        adapter = new PacketBuffer;
    if (handler == NULL)
    {
        handler = new PacketHandler;
        adapter->SetHandler(handler);
    }
}

//! Finalize
void Destroy()
{
    if (adapter)
        delete adapter;
    if (handler)
        delete handler;
}

//! Recv
int WSAAPI RecvInternal(int a, char* b, int c, int d)
{
    int result = pfnRecv(a, b, c, d);
    if (result > 0)
        result = adapter->Recv(b, result);

    return result;
}

//! Attach
void Attach()
{
    if (ULONG addr = Context.Addr[ADDR_WS2RECV])
    {
        while (*reinterpret_cast<ULONG*>(addr)==0)
            SleepEx(100, TRUE);

        pfnRecv = reinterpret_cast<LPFN_RECV>(InterlockedExchangePointer(reinterpret_cast<PVOID>(addr), RecvInternal));
    }

    DbgPrintW(L"Attach recv@ws2 %08X -> %08X\n", pfnRecv, RecvInternal);
}


};
