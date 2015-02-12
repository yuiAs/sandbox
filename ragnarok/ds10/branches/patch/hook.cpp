#include "client.hpp"
#include "../config.hpp"
#include "../pe_operate.hpp"

extern PEOp* pe;

////////////////////////////////////////////////////////////////////////////////


namespace client
{
	void *pfn_k32_cfile = NULL;
	void *pfn_u32_msgbox = NULL;
	void *pfn_a32_regckey = NULL;

////////////////////////////////////////////////////////////////////////////////

// MessageBoxA@user32

int WINAPI u32_msgbox(HWND a, LPCSTR b, LPCSTR c, UINT d)
{
	if (c == MB_OK)
	{
		dbgprintf(1, "[%s] %s\n", b, c);
		return TRUE;
	}

	typedef int (WINAPI *U32_MSGBOX)(HWND, LPCSTR, LPCSTR, UINT);
	return reinterpret_cast<U32_MSGBOX>(pfn_u32_msgbox)(a, b, c, d);
}

////////////////////////////////////////////////////////////////////////////////

// Registry

typedef LONG (WINAPI *A32_REGCKEY_EX_A)(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);

bool _reg_checksubkey(const char* subkeyname)
{
	const char* def = "Software\\Gravity Soft\\Ragnarok\\ShortcutItem\\";
	return memcmp(def, subkeyname, strlen(def))==0;
}

LONG _reg_createsubkey(HKEY a, const char* subkeyname, REGSAM c, LPSECURITY_ATTRIBUTES d, PHKEY e, LPDWORD f, DWORD aid)
{
	char buf[MAX_PATH];
	_snprintf_s(buf, MAX_PATH, MAX_PATH-1, "%s\\%08X", subkeyname, aid);

	return reinterpret_cast<A32_REGCKEY_EX_A>(pfn_a32_regckey)(a, buf, 0, NULL, REG_OPTION_NON_VOLATILE, c, d, e, f);
}

LONG WINAPI a32_regckey(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i)
{
	__try
	{
		DWORD aid = *reinterpret_cast<DWORD*>(address(AD_AID));

		if (_reg_checksubkey(b))
			return _reg_createsubkey(a, b, f, g, h, i, aid);
	}
	__except (::GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
	{
	}

	return reinterpret_cast<A32_REGCKEY_EX_A>(pfn_a32_regckey)(a, b, c, d, e, f, g, h, i);

/*
	if (::IsBadReadPtr(reinterpret_cast<const void*>(address(AD_AID)), sizeof(DWORD)) == 0)
	{
		if (_reg_checksubkey(b))
			return _reg_createsubkey(a, b, f, g, h, i, *reinterpret_cast<DWORD*>(address(AD_AID)));
	}

	return reinterpret_cast<A32_REGCKEY_EX_A>(pfn_a32_regckey)(a, b, c, d, e, f, g, h, i);
*/
}

LONG WINAPI a32_regokey(HKEY a, LPCSTR b, DWORD c, REGSAM d, PHKEY e)
{
	return a32_regckey(a, b, 0, NULL, REG_OPTION_NON_VOLATILE, d, NULL, e, NULL);
}

////////////////////////////////////////////////////////////////////////////////

// CreateFileA@kernel32

HANDLE WINAPI k32_cfile(LPCSTR a, DWORD b, DWORD c, LPSECURITY_ATTRIBUTES d, DWORD e, DWORD f, HANDLE g)
{
	typedef HANDLE (WINAPI *K32_CFILE_A)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

	// CREATE_ALWAYS‚Í‚»‚Ì‚Ü‚Ü’Ê‚·
	if (e == CREATE_ALWAYS)
		return reinterpret_cast<K32_CFILE_A>(pfn_k32_cfile)(a, b, c, d, e, f, g);

	// data\...
	if ((a[0]==0x64) && (a[4]==0x5C))
	{
		wchar_t filename[MAX_PATH];
		::MultiByteToWideChar(949, NULL, a, -1, filename, MAX_PATH);
		return ::CreateFileW(filename, b, c, d, e, f, g);
	}

	return reinterpret_cast<K32_CFILE_A>(pfn_k32_cfile)(a, b, c, d, e, f, g);
}

////////////////////////////////////////////////////////////////////////////////

// initialize

void apihook()
{
	using namespace config;

	if (enabled(FS_UNICODE))
		pfn_k32_cfile = pe->jmp_rewrite("KERNEL32.dll", "CreateFileA", k32_cfile);

	if (enabled(SC_REGEX))
	{
		pfn_a32_regckey = pe->jmp_rewrite("ADVAPI32.dll", "RegCreateKeyExA", a32_regckey);
		pe->jmp_rewrite("ADVAPI32.dll", "RegOpenKeyExA", a32_regokey);
	}

	if (enabled(EX_SKIPMSG))
		pe->jmp_rewrite("USER32.dll", "MessageBoxA", u32_msgbox);
}


void apihook_cleanup()
{
}


};	// namespace client
