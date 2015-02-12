#include "net.h"
#include "../client.h"

////////////////////////////////////////////////////////////////////////////////

namespace net
{
	static Client* m_cl = NULL;
	static ROPacketParser* m_parser = NULL;

	static void* m_recv = NULL;
	static void* m_send = NULL;

////////////////////////////////////////////////////////////////////////////////


int WSAAPI api_recv(SOCKET s, char* buf, int len, int flags)
{
	typedef int (WSAAPI *API_RECV)(SOCKET, char*, int, int);

	int result = reinterpret_cast<API_RECV>(m_recv)(s, buf, len, flags);
	if (result != SOCKET_ERROR)
		m_parser->recv(buf, result);

	return result;
}


int WSAAPI api_send(SOCKET s, const char* buf, int len, int flags)
{
	int result1 = m_parser->send(buf, len);
	if (result1 != 0)
		return result1;

//	int result2 = m_parser->exec_sendque(s, buf, len, flags);
//	if (result2 > result1)
//		return result1;

	typedef int (WSAAPI *API_SEND)(SOCKET, const char*, int, int);
	return reinterpret_cast<API_SEND>(m_send)(s, buf, len, flags);
}

////////////////////////////////////////////////////////////////////////////////


void init(void* client)
{
	m_cl = reinterpret_cast<Client*>(client);
	m_parser = new ROPacketParser;

	// APIHook

	if (DWORD _tmp = m_cl->GetAddress(AD_RECV))
	{
		m_recv = reinterpret_cast<void*>(*reinterpret_cast<DWORD*>(_tmp));
		*reinterpret_cast<DWORD*>(_tmp) = reinterpret_cast<DWORD>(api_recv);
	}

	if (DWORD _tmp = m_cl->GetAddress(AD_SEND))
	{
		m_send = reinterpret_cast<void*>(*reinterpret_cast<DWORD*>(_tmp));
		*reinterpret_cast<DWORD*>(_tmp) = reinterpret_cast<DWORD>(api_send);
	}

	m_parser->init(m_cl->GetAddress(AD_NETBASE));
	m_parser->init_api(m_recv, m_send);
}


void destroy()
{
	if (m_parser)
		delete m_parser;
}


};	// namespace net