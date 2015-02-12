#include "netcode.hpp"
#include "../peimage.hpp"
#include "packet.hpp"

extern PEImage* pe;
extern CPacket* packet;

void *pfn_ws2_connect, *pfn_ws2_recv, *pfn_ws2_send, *pfn_ws2_inet_addr;

////////////////////////////////////////////////////////////////////////////////


int WSAAPI ws2_connect(SOCKET s, const struct sockaddr FAR * name, int namelen)
{
	typedef int (WSAAPI *WS2_CONNECT)(SOCKET, const struct sockaddr FAR *, int);
	return reinterpret_cast<WS2_CONNECT>(pfn_ws2_connect)(s, name, namelen);
}


int WSAAPI ws2_recv(SOCKET s, char *buf, int len, int flags)
{
	typedef int (WSAAPI *WS2_RECV)(SOCKET, char*, int, int);
	int result = reinterpret_cast<WS2_RECV>(pfn_ws2_recv)(s, buf, len, flags);

	if (result != SOCKET_ERROR)
		result += packet->recvCall(reinterpret_cast<BYTE*>(buf), result);

	return result;
}


int WSAAPI ws2_send(SOCKET s, const char *buf, int len, int flags)
{
	int result = packet->sendCall(reinterpret_cast<BYTE*>(const_cast<char*>(buf)), len);
	if (result != 0)
		return result;

	typedef int (WSAAPI *WS2_SEND)(SOCKET, const char*, int, int);
	return reinterpret_cast<WS2_SEND>(pfn_ws2_send)(s, buf, len, flags);
}


unsigned long WSAAPI ws2_inet_addr(const char* cp)
{
	typedef unsigned long (WSAAPI *WS2_INET_ADDR)(const char*);
	return reinterpret_cast<WS2_INET_ADDR>(pfn_ws2_inet_addr)(cp);
}

////////////////////////////////////////////////////////////////////////////////


void netcode::start()
{
	// jumptableèëÇ´ä∑Ç¶

//	pfn_ws2_connect = pe->rewriteFF25(_T("ws2_32.dll"), "connect", ws2_connect);
	DbgPrint(_T("pfn_ws2_connect=%08X"), pfn_ws2_connect);
//	pfn_ws2_inet_addr = pe->rewriteFF25(_T("ws2_32.dll"), "inet_addr", ws2_inet_addr);
	DbgPrint(_T("pfn_ws2_inet_addr=%08X"), pfn_ws2_inet_addr);

	// bufferèëÇ´ä∑Ç¶

	DWORD addr_recv = 0;
	pfn_ws2_recv = reinterpret_cast<DWORD*>(addr_recv);
//	pe->attachMemory(reinterpret_cast<void*>(addr_recv), ws2_recv, sizeof(DWORD));
	DbgPrint(_T("addr_recv=%08X pfn_ws2_recv=%08X"), addr_recv, pfn_ws2_recv);

	DWORD addr_send = 0;
	pfn_ws2_send = reinterpret_cast<DWORD*>(addr_send);
//	pe->attachMemory(reinterpret_cast<void*>(addr_send), ws2_send, sizeof(DWORD));
	DbgPrint(_T("addr_send=%08X pfn_ws2_send=%08X"), addr_send, pfn_ws2_send);
}
