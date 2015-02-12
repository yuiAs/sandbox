#include <winsock2.h>
#include <windows.h>
#include "rPacket.h"
#include "intrnl.h"


typedef int (WSAAPI *PTR_RECV)(int, char*, int, int);
typedef int (WSAAPI *PTR_SEND)(int, const char*, int, int);

PVOID g_Recv=NULL, g_Send=NULL;
ROPacket* g_Packet = NULL;

////////////////////////////////////////////////////////////////////////////////


int WSAAPI InjectRecv(int a, char* b, int c, int d)
{
	int Result = reinterpret_cast<PTR_RECV>(g_Recv)(a, b, c, d);
	if (Result > 0)
		Result = g_Packet->Recv(b, Result);

	return Result;
}

int WSAAPI InjectSend(int a, const char* b, int c, int d)
{
	int Result = g_Packet->Send(const_cast<char*>(b), c);
	if (Result > 0)
		return Result;

	return reinterpret_cast<PTR_SEND>(g_Send)(a, b, c, d);
}

void InjectWinsock2()
{
	ULONG IntrnlRecv = g_Intrnl.Addr[IA_RECV];
	ULONG IntrnlSend = g_Intrnl.Addr[IA_SEND];

	if (IntrnlRecv)
		g_Recv = InterlockedExchangePointer(reinterpret_cast<PVOID>(IntrnlRecv), reinterpret_cast<PVOID>(InjectRecv));
	if (IntrnlSend && (g_Intrnl.DisableSend==false))
		g_Send = InterlockedExchangePointer(reinterpret_cast<PVOID>(IntrnlSend), reinterpret_cast<PVOID>(InjectSend));
}

////////////////////////////////////////////////////////////////////////////////


void InitNetwork()
{
	InjectWinsock2();

	if (g_Packet == NULL)
	{
		g_Packet = new ROPacket;
		g_Packet->Init(g_Intrnl.Addr[IA_PLT]);
	}
}

void DestroyNetwork()
{
	if (g_Packet)
	{
		delete g_Packet;
		g_Packet = NULL;
	}
}
