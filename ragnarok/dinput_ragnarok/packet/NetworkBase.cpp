#include "NetworkBase.hpp"


CNetworkBase* CNetworkBase::m_this = NULL;

////////////////////////////////////////////////////////////////////////////////


CNetworkBase::CNetworkBase()
{
	m_this = this;
	m_pfnSend = m_pfnRecv = m_pfnConnect = m_pfnInetAddr = 0;

	initialize();
}


CNetworkBase::~CNetworkBase()
{
}

////////////////////////////////////////////////////////////////////////////////


void CNetworkBase::initialize()
{
	HMODULE hModule = ::LoadLibrary(_T("ws2_32.dll"));

	m_pfnSend = ::GetProcAddress(hModule, _T("send"));
	m_pfnRecv = ::GetProcAddress(hModule, _T("recv"));
	m_pfnConnect = ::GetProcAddress(hModule, _T("connect"));
	m_pfnInetAddr = ::GetProcAddress(hModule, _T("inet_addr"));

	::FreeLibrary(hModule);
}

////////////////////////////////////////////////////////////////////////////////

// ws2_32::inet_addr

unsigned long WSAAPI CNetworkBase::ws2_inet_addr(const char* cp)
{
	typedef unsigned long (WSAAPI *WS2_INETADDR)(const char*);
	return reinterpret_cast<WS2_INETADDR>(m_pfnInetAddr)(cp);
}

// ws2_32::connect

int WSAAPI CNetworkBase::ws2_connect(SOCKET s, const struct sockaddr FAR * name, int namelen)
{
	typedef int (WSAAPI *WS2_CONNECT)(SOCKET, const struct sockaddr FAR *, int);
	return reinterpret_cast<WS2_CONNECT>(m_pfnConnect)(s, name, namelen);
}

// ws2_32::recv

int WSAAPI CNetworkBase::ws2_recv(SOCKET s, char *buf, int len, int flags)
{
	typedef int (WSAAPI *WS2_RECV)(SOCKET, char*, int, int);
	int result = reinterpret_cast<WS2_RECV>(m_pfnRecv)(s, buf, len, flags);

	if (result != SOCKET_ERROR)
		result += recv(reinterpret_cast<unsigned char*>(buf), result);
    
	return result;
}

// ws2_32::send

int WSAAPI CNetworkBase::ws2_send(SOCKET s, const char *buf, int len, int flags)
{
	int result = send(reinterpret_cast<unsigned char*>(const_cast<char*>(buf)), 0);
	if (result != 0)
		return result;

	typedef int (WSAAPI *WS2_SEND)(SOCKET, const char*, int, int);
	return reinterpret_cast<WS2_SEND>(m_pfnSend)(s, buf, len, flags);
}

////////////////////////////////////////////////////////////////////////////////


unsigned long WSAAPI CNetworkBase::_inet_addr(const char* cp)
{
	CNetworkBase* c = CNetworkBase::m_this;
	return c->ws2_inet_addr(cp);
}


int WSAAPI CNetworkBase::_connect(SOCKET s, const struct sockaddr FAR * name, int namelen)
{
	CNetworkBase* c = CNetworkBase::m_this;
	return c->ws2_connect(s, name, namelen);
}


int WSAAPI CNetworkBase::_recv(SOCKET s, char* buf, int len, int flags)
{
	CNetworkBase* c = CNetworkBase::m_this;
	return c->ws2_recv(s, buf, len, flags);
}


int WSAAPI CNetworkBase::_send(SOCKET s, const char* buf, int len, int flags)
{
	CNetworkBase* c = CNetworkBase::m_this;
	return c->ws2_send(s, buf, len, flags);
}
