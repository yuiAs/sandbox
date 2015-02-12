#ifndef NETWORK_HPP_92B3BB98_35B4_4033_8E43_15B4F40A135D
#define NETWORK_HPP_92B3BB98_35B4_4033_8E43_15B4F40A135D

#include <winsock2.h>

////////////////////////////////////////////////////////////////////////////////


namespace network
{
	static void *pfn_ws2_recv = NULL;
	static void *pfn_ws2_send = NULL;

	// nethook.cpp

	void init(DWORD netbase, DWORD recv, DWORD send);
	void fin();

	// network.cpp

	int rbuf_send(u_char* buf, int rest);
	void rbuf_push(u_char* buf, int buflen);
	void rbuf_clear();

	void sbuf_send(SOCKET s, int flags);
	void sbuf_push(u_char* buf, int buflen);
	void sbuf_clear();
};


#endif	// #ifndef NETWORK_HPP
