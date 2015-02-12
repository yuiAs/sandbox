#include "client.h"
#include "dbgprint.h"

////////////////////////////////////////////////////////////////////////////////


Client::Client()
{
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
}


Client::~Client()
{
	::CoUninitialize();
}

////////////////////////////////////////////////////////////////////////////////


void Client::init()
{
	PEOp::init(::GetModuleHandle(NULL));
	
	memset(m_address, 0, sizeof(m_address));
	m_importTable.clear();
}


void Client::analysis()
{
	search_APIJmpTable(0x1000);

	search_netbase(0x40);
	search_send(0x40);
	search_recv(0x40);
	search_gamebase(0x10);
	search_drawbase(0x20);
	search_aid(0x40);
	search_charname(0x20, 0x40);
	search_msgTable(0x20);
	search_zoneParser(0x20);

	dbgprintf(0, "AD_NETBASE=%08X\n", m_address[AD_NETBASE]);
	dbgprintf(0, "AD_SEND=%08X\n", m_address[AD_SEND]);
	dbgprintf(0, "AD_RECV=%08X\n", m_address[AD_RECV]);
	dbgprintf(0, "AD_AID=%08X\n", m_address[AD_AID]);
	dbgprintf(0, "AD_CHARNAME=%08X\n", m_address[AD_CHARNAME]);
	dbgprintf(0, "AD_ACTORBASE=%08X\n", m_address[AD_ACTORBASE]);
	dbgprintf(0, "AD_DRAWBASE=%08X\n", m_address[AD_DRAWBASE]);
	dbgprintf(0, "AD_GAMEBASE=%08X\n", m_address[AD_GAMEBASE]);
	dbgprintf(0, "AD_ZONE_JMPTBL=%08X\n", m_address[AD_ZONE_JMPTBL]);
	dbgprintf(0, "AD_CALL_CLPRT=%08X\n", m_address[AD_CALL_CLPRT]);
	dbgprintf(0, "AD_CALL_MSGTBL=%08X\n", m_address[AD_CALL_MSGTBL]);
	dbgprintf(0, "AD_CALL_ZP0196=%08X\n", m_address[AD_CALL_ZP0196]);
}


void Client::hook()
{
	if (::GetPrivateProfileInt(_T("client"), _T("multiple_sci"), 0, _T("./ijl15.ini")))
	{
		apihook(_T("ADVAPI32.dll"), "RegCreateKeyExA", dummy_RegCreateKeyExA);
		apihook(_T("ADVAPI32.dll"), "RegOpenKeyExA", dummy_RegOpenKeyExA);
	}

	if (::GetPrivateProfileInt(_T("client"), _T("skip_msgbox"), 0, _T("./ijl15.ini")))
		apihook(_T("USER32.dll"), "MessageBoxA", dummy_MessageBoxA);
}

////////////////////////////////////////////////////////////////////////////////


void* Client::apihook(LPCTSTR module, LPCSTR name, void* func)
{
	void* address = ::GetProcAddress(::GetModuleHandle(module), name);

	std::map<DWORD, DWORD>::const_iterator it = m_importTable.find(reinterpret_cast<DWORD>(address));
	if (it != m_importTable.end())
	{
		dbgprintf(0, "%s.%s %08X %08X\n", module, name, it->first, it->second);
		memcpy_f(reinterpret_cast<void*>(it->second), &func, sizeof(DWORD));
		return reinterpret_cast<void*>(address);
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////


void Client::unprotect_sakray()
{
	// old 00050505050102030505050504
	// new 00050105050502030505050504
	//     0 1 2 3 4 5 6 7 8 9 A B C

	BYTE code[] = {
		0x00, 0x05, 0x01, 0x05, 0x05, 0x05, 0x02, 0x03, 0x05, 0x05, 0x05, 0x05, 0x04
	};

	DWORD address = findbin(code, sizeof(code));
	if (address == 0)
		return;

	dbgprintf(0, "country_code=%08X\n", address);

	BYTE m = 0x05;
	memcpy_f(reinterpret_cast<void*>(address+2), &m, sizeof(BYTE));
}

////////////////////////////////////////////////////////////////////////////////


void Client::WritePrivateProfileHex(LPCTSTR lpAppName, LPCTSTR lpKeyName, DWORD val, LPCTSTR lpFileName)
{
	TCHAR buf[16];
#if _MSC_VER >= 1400
	_snprintf_s(buf, 16, 16-1, "0x%08X", val);
#else
	_snprintf(buf, 16, "0x%08X", val);
#endif
	::WritePrivateProfileString(lpAppName, lpKeyName, buf, lpFileName);
}