#include "Packet.hpp"
#include "../apiHook.hpp"


CPacket* CPacket::m_this = 0;

////////////////////////////////////////////////////////////////////////////////


CPacket::CPacket()
{
	m_this = this;

	extern CRagnarok* core;
	m_core = core;
	m_parser = new CPacketParser;

	m_pfnGetProcAddress = 0;

	::ZeroMemory(&m_current_addr, sizeof(in_addr));

	initialize();
}


CPacket::~CPacket()
{
	delete m_parser;
}

////////////////////////////////////////////////////////////////////////////////


void CPacket::initialize()
{
	apihook();
}

////////////////////////////////////////////////////////////////////////////////


void CPacket::apihook()
{
	APIHook* api = new APIHook;

	DWORD address = m_core->getAddr(AD_GETPROCADDR);

	m_pfnGetProcAddress = api->HookAPICall(_T("KERNEL32.dll"), _T("GetProcAddress"), _GetProcAddress);
	m_pfnConnect = api->HookAPICallOrdinal(_T("ws2_32.dll"), 4, _connect);
	m_pfnInetAddr = api->HookAPICallOrdinal(_T("ws2_32.dll"), 11, _inet_addr);

	delete api;
}

////////////////////////////////////////////////////////////////////////////////


// ws2_32::inet_addr

unsigned long WSAAPI CPacket::ws2_inet_addr(const char* cp)
{
	typedef unsigned long (WSAAPI *WS2_INETADDR)(const char*);
	unsigned long sa = reinterpret_cast<WS2_INETADDR>(m_pfnInetAddr)(cp);

	// inet_addrで変換できない場合、INADDR_NONEが返る
	if (sa == INADDR_NONE)
	{
		struct hostent* h;

		// 動的確保された<address>で指定された文字列bufferのアドレスを求める
		// 確保タイミング的に事前取得は難しい?

		if (DWORD dwAccSvIP = m_core->getAddr(AD_ACCSVIP))
		{
			DWORD d;
			::CopyMemory(&d, reinterpret_cast<const void*>(dwAccSvIP), sizeof(DWORD));
			h = gethostbyname(reinterpret_cast<TCHAR*>(d));
		}
		else
			h = gethostbyname(cp);

		if (h != NULL)
			sa = *reinterpret_cast<unsigned long*>(h->h_addr_list[0]);
	}

	m_core->output(_T("%s=%08X"), cp, sa);

	return sa;
}

// ws2_connect

int WSAAPI CPacket::ws2_connect(SOCKET s, const struct sockaddr FAR * name, int namelen)
{
	in_addr sin_addr = reinterpret_cast<const sockaddr_in*>(name)->sin_addr;

	// 127.0.0.1へ繋ぐ場合、直前のaddressを使う

	if (sin_addr.S_un.S_addr == 0x0100007F)
		const_cast<sockaddr_in*>(reinterpret_cast<const sockaddr_in*>(name))->sin_addr = m_current_addr;
	else
		m_current_addr = sin_addr;

	m_parser->setCurrentIP(m_current_addr.S_un.S_addr);

	// table構築

	if (m_parser->isTable() == false)
	{
		DWORD addrNet = reinterpret_cast<DWORD>(name);
		m_core->output(_T("networkbase=%08X +0x08"), addrNet-0x08);
		m_parser->buildTable(addrNet-0x08, 0x00);
	}


	typedef int (WSAAPI *WS2_CONNECT)(SOCKET, const struct sockaddr FAR *, int);
	return reinterpret_cast<WS2_CONNECT>(m_pfnConnect)(s, name, namelen);
}

////////////////////////////////////////////////////////////////////////////////


FARPROC CPacket::k32_gpa(HMODULE hModule, LPCSTR lpProcName)
{
	typedef FARPROC (WINAPI *LPGETPROCADDRESS)(HMODULE, LPCSTR);
	FARPROC pfnReturn = reinterpret_cast<LPGETPROCADDRESS>(m_pfnGetProcAddress)(hModule, lpProcName);

	if (hModule != ::GetModuleHandle(_T("ws2_32.dll")))
		return pfnReturn;

	if (::lstrcmpi(_T("recv"), lpProcName) == 0)
	{
		m_pfnRecv = pfnReturn;
		return reinterpret_cast<FARPROC>(_recv);
	}

	if (::lstrcmpi(_T("send"), lpProcName) == 0)
	{
		m_pfnSend = pfnReturn;
		return reinterpret_cast<FARPROC>(_send);
	}

	return pfnReturn;
}

