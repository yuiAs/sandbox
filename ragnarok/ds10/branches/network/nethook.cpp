#include "../common.h"
#include "network.hpp"
#include "packet.hpp"
#include "../config.hpp"

////////////////////////////////////////////////////////////////////////////////


namespace network
{
	CPacket* packet = NULL;

////////////////////////////////////////////////////////////////////////////////

// recv@ws2_32

int WSAAPI ws2_recv(SOCKET s, char *buf, int len, int flags)
{
	typedef int (WSAAPI *WS2_RECV)(SOCKET, char*, int, int);
	int result = reinterpret_cast<WS2_RECV>(pfn_ws2_recv)(s, buf, len, flags);

	if (result != SOCKET_ERROR)
	{
		packet->recv(buf, result);
		result += rbuf_send(reinterpret_cast<u_char*>(buf)+result, len-result);
	}

	return result;
}

// send@ws2_32

int WSAAPI ws2_send(SOCKET s, const char *buf, int len, int flags)
{
#ifdef _EXTENTION
	sbuf_send(s, flags);
#endif

	int result = packet->send(buf, len);
	if (result != 0)
		return result;

	typedef int (WSAAPI *WS2_SEND)(SOCKET, const char*, int, int);
	return reinterpret_cast<WS2_SEND>(pfn_ws2_send)(s, buf, len, flags);
}

////////////////////////////////////////////////////////////////////////////////

// initialize

void init(DWORD netbase, DWORD recv, DWORD send)
{
	if (recv)
	{
		pfn_ws2_recv = reinterpret_cast<void*>(*reinterpret_cast<DWORD*>(recv));
		*reinterpret_cast<DWORD*>(recv) = reinterpret_cast<DWORD>(ws2_recv);

		rbuf_clear();
	}

	if (send)
	{
		pfn_ws2_send = reinterpret_cast<void*>(*reinterpret_cast<DWORD*>(send));
		*reinterpret_cast<DWORD*>(send) = reinterpret_cast<DWORD>(ws2_send);

		sbuf_clear();
	}

	{
		DWORD address = config::get_n("packet", "plt_address");
		bool as = false;
		
		if (address == 0)
		{
			address = netbase;
			as = true;
		}

		packet = new CPacket;
		packet->initialize(address, as);
	}

	dbgprintf(0, "recv_a=%08X recv_o=%08X recv_m=%08X send_a=%08X send_o=%08X send_m=%08X\n", recv, pfn_ws2_recv, ws2_recv, send, pfn_ws2_send, ws2_send);
}

// finalize

void fin()
{
	if (packet)
	{
		delete packet;
		packet = NULL;
	}

	rbuf_clear();
	sbuf_clear();
}


};	// namespace network

