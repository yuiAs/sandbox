#include "client.h"
#include <comutil.h>
#include "dbgprint.h"

extern Client* cl;

////////////////////////////////////////////////////////////////////////////////


int WINAPI Client::dummy_MessageBoxA(HWND a, LPCSTR b, LPCSTR c, UINT d)
{
	return cl->api_MessageBoxA(a, b, c, d);
}

LONG WINAPI Client::dummy_RegCreateKeyExA(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i)
{
	return cl->api_RegCreateKeyExA(a, b, c, d, e, f, g, h, i);
}

LONG WINAPI Client::dummy_RegOpenKeyExA(HKEY a, LPCSTR b, DWORD c, REGSAM d, PHKEY e)
{
	return cl->api_RegOpenKeyExA(a, b, c, d, e);
}

////////////////////////////////////////////////////////////////////////////////


int Client::api_MessageBoxA(HWND a, LPCSTR b, LPCSTR c, UINT d)
{
	dbgprintf(5, "[%08X] %s : %s\n", ::GetTickCount(), c, b);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////


bool Client::is_shortcutitem(const char* subkey)
{
	const char* tmp = "Software\\Gravity Soft\\Ragnarok\\ShortcutItem\\";
	return memcmp(tmp, subkey, strlen(tmp))==0;
}


LONG Client::api_RegCreateKeyExW(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i)
{
	// http://msdn.microsoft.com/library/en-us/sysinfo/base/regcreatekeyex.asp?frame=true
	// lpClass (d)
	// The class (object type) of this key.
	// This parameter may be ignored. This parameter can be NULL.

	_bstr_t _key(b);
	return ::RegCreateKeyExW(a, _key, c, NULL, e, f, g, h, i);
}


LONG Client::api_RegCreateKeyExA(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i)
{
	__try
	{
		if (is_shortcutitem(b))
		{
			char buf[MAX_PATH];	// w2kˆÈ‘O‚Ì§ŒÀ?
#if _MSC_VER >= 1400
			_snprintf_s(buf, MAX_PATH, MAX_PATH-1, "%s\\%08X", b, *reinterpret_cast<DWORD*>(m_address[AD_AID]));
#else
			_snprintf(buf, MAX_PATH, "%s\\%08X", b, *reinterpret_cast<DWORD*>(m_address[AD_AID]));
#endif
			return api_RegCreateKeyExW(a, buf, c, NULL, e, f, g, h, i);
		}
	}
	__except (::GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
	{
	}

	return api_RegCreateKeyExW(a, b, c, NULL, e, f, g, h, i);
}


LONG Client::api_RegOpenKeyExA(HKEY a, LPCSTR b, DWORD c, REGSAM d, PHKEY e)
{
	return api_RegCreateKeyExA(a, b, 0, NULL, REG_OPTION_NON_VOLATILE, d, NULL, e, NULL);
}
