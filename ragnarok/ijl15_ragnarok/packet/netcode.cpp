#include "netcode.hpp"
#include "../dllconf.hpp"
#include "../peimage.hpp"
#include "packet.hpp"

extern PEImage* pe;
extern DLLConf* conf;
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
	// jumptable‘‚«Š·‚¦

//	pfn_ws2_connect = pe->rewriteFF25(_T("ws2_32.dll"), "connect", ws2_connect);
//	DbgPrint(_T("pfn_ws2_connect=%08X"), pfn_ws2_connect);
//	pfn_ws2_inet_addr = pe->rewriteFF25(_T("ws2_32.dll"), "inet_addr", ws2_inet_addr);
//	DbgPrint(_T("pfn_ws2_inet_addr=%08X"), pfn_ws2_inet_addr);

	// buffer‘‚«Š·‚¦

	DWORD addr_recv = conf->get_addr(ADDR_RECV);
	if (addr_recv)
	{
		pfn_ws2_recv = reinterpret_cast<void*>(*reinterpret_cast<DWORD*>(addr_recv));
		*reinterpret_cast<DWORD*>(addr_recv) = reinterpret_cast<DWORD>(ws2_recv);
	}

	DWORD addr_send = conf->get_addr(ADDR_SEND);
	if (addr_send)
	{
		pfn_ws2_send = reinterpret_cast<void*>(*reinterpret_cast<DWORD*>(addr_send));
		*reinterpret_cast<DWORD*>(addr_send) = reinterpret_cast<DWORD>(ws2_send);
	}

	DWORD address = conf->get_addr(ADDR_NETBASE);
	packet->buildTable(address);
}
