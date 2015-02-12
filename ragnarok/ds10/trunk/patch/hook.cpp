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
	void *pfn_imm_getclw = NULL;
	void *pfn_g32_selectobj = NULL;

	HFONT m_localFont = NULL;
	HFONT m_ansiFont = NULL;

////////////////////////////////////////////////////////////////////////////////

// SelectObject@gdi32

HGDIOBJ WINAPI g32_selectobj(HDC a, HGDIOBJ b)
{
	typedef HGDIOBJ (WINAPI *G32_SELECTOBJ)(HDC, HGDIOBJ);

	if (::GetObjectType(b) != OBJ_FONT)
		return reinterpret_cast<G32_SELECTOBJ>(pfn_g32_selectobj)(a, b);
	if (::GetStockObject(SYSTEM_FONT) == b)
		return reinterpret_cast<G32_SELECTOBJ>(pfn_g32_selectobj)(a, b);

//	if (b == m_localFont)
//		return reinterpret_cast<G32_SELECTOBJ>(pfn_g32_selectobj)(a, b);
//	if (b == m_ansiFont)
//		return reinterpret_cast<G32_SELECTOBJ>(pfn_g32_selectobj)(a, b);

	LOGFONT lf;
	::GetObjectA(b, sizeof(LOGFONT), &lf);

	if (lf.lfCharSet == ANSI_CHARSET)
		return reinterpret_cast<G32_SELECTOBJ>(pfn_g32_selectobj)(a, m_ansiFont);
	else
	{
/*
		fix_hfont(AD_FONT, &m_localFont);
		::DeleteObject(b);

		return reinterpret_cast<G32_SELECTOBJ>(pfn_g32_selectobj)(a, b);
*/
		return reinterpret_cast<G32_SELECTOBJ>(pfn_g32_selectobj)(a, m_localFont);
	}

	return ::GetCurrentObject(a, OBJ_FONT);
}

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
//#if _MSC_VER >= 1400
	_snprintf_s(buf, MAX_PATH, MAX_PATH-1, "%s\\%08X", subkeyname, aid);
//#else
//	_snprintf(buf, MAX_PATH, "%s\\%08X", subkeyname, aid);
//#endif

	return reinterpret_cast<A32_REGCKEY_EX_A>(pfn_a32_regckey)(a, buf, 0, NULL, REG_OPTION_NON_VOLATILE, c, d, e, f);
}

LONG WINAPI a32_regckey(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i)
{
	if (::IsBadReadPtr(reinterpret_cast<const void*>(address(AD_AID)), sizeof(DWORD)) == 0)
	{
		if (_reg_checksubkey(b))
			return _reg_createsubkey(a, b, f, g, h, i, *reinterpret_cast<DWORD*>(address(AD_AID)));
	}

	return reinterpret_cast<A32_REGCKEY_EX_A>(pfn_a32_regckey)(a, b, c, d, e, f, g, h, i);
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

// CreateFontA@gdi32

HFONT WINAPI g32_cfont(int a, int b, int c, int d, int e, DWORD f, DWORD g, DWORD h, DWORD i, DWORD j, DWORD k, DWORD l, DWORD m, LPCSTR n)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	lf.lfHeight = a;
	lf.lfWidth = b;
	lf.lfEscapement = c;
	lf.lfOrientation = d;
	lf.lfWeight = e;
	lf.lfItalic = static_cast<BYTE>(f);
	lf.lfUnderline = static_cast<BYTE>(g);
	lf.lfStrikeOut = static_cast<BYTE>(h);
	lf.lfCharSet = static_cast<BYTE>(i);
	lf.lfOutPrecision = static_cast<BYTE>(j);
	lf.lfClipPrecision = static_cast<BYTE>(k);
	lf.lfQuality = static_cast<BYTE>(l);
	lf.lfPitchAndFamily = static_cast<BYTE>(m);
	memcpy(lf.lfFaceName, n, LF_FACESIZE);

	using namespace config;

	if (i == ANSI_CHARSET)
	{
		if (enabled(FONTA_ENABLE))
		{
			lf.lfHeight = get_n(FONTA_SIZE);
			lf.lfWeight = get_n(FONTA_WEIGHT);
			lf.lfQuality = static_cast<BYTE>(get_n(FONTA_QUALITY));
			get_s(FONTA_NAME, lf.lfFaceName, LF_FACESIZE);
		}
	}
	else
	{
		if (enabled(FONTL_ENABLE))
		{
			lf.lfHeight = get_n(FONTL_SIZE);
			lf.lfWeight = get_n(FONTL_WEIGHT);
			lf.lfQuality = static_cast<BYTE>(get_n(FONTL_QUALITY));
			lf.lfCharSet = static_cast<BYTE>(get_n(FONTL_CHARSET));
			get_s(FONTL_NAME, lf.lfFaceName, LF_FACESIZE);
		}
	}

	return ::CreateFontIndirectA(&lf);
}

////////////////////////////////////////////////////////////////////////////////

// initialize

void font_CreateFontA()
{
	pe->jmp_rewrite("GDI32.dll", "CreateFontA", g32_cfont);
}

void font_SelectObject()
{
	using namespace config;

	LOGFONT lf;

	if (enabled(FONTA_ENABLE))
	{
		memset(&lf, 0, sizeof(LOGFONT));

		lf.lfHeight = get_n(FONTA_SIZE);
		lf.lfWeight = get_n(FONTA_WEIGHT);
		lf.lfQuality = static_cast<BYTE>(get_n(FONTA_QUALITY));
		get_s(FONTA_NAME, lf.lfFaceName, LF_FACESIZE);

		m_ansiFont = ::CreateFontIndirectA(&lf);
	}

	if (enabled(FONTL_ENABLE))
	{
		memset(&lf, 0, sizeof(LOGFONT));

		lf.lfHeight = get_n(FONTL_SIZE);
		lf.lfWeight = get_n(FONTL_WEIGHT);
		lf.lfQuality = static_cast<BYTE>(get_n(FONTL_QUALITY));
		lf.lfCharSet = static_cast<BYTE>(get_n(FONTL_CHARSET));
		get_s(FONTL_NAME, lf.lfFaceName, LF_FACESIZE);

		m_localFont = ::CreateFontIndirectA(&lf);
	}

	pfn_g32_selectobj = pe->jmp_rewrite("GDI32.dll", "SelectObject", g32_selectobj);
}

void apihook()
{
	using namespace config;

	if (enabled(FS_UNICODE))
		pfn_k32_cfile = pe->jmp_rewrite("KERNEL32.dll", "CreateFileA", k32_cfile);

	if (enabled(FONTA_ENABLE) || enabled(FONTL_ENABLE))
	{
		switch (get_n(FONT_API))
		{
			case 0:
				font_CreateFontA();
				break;
			case 1:
				font_SelectObject();
				break;
			default:
				break;
		}
	}

	if (enabled(SC_REGEX))
	{
		pfn_a32_regckey = pe->jmp_rewrite("ADVAPI32.dll", "RegCreateKeyExA", a32_regckey);
		pe->jmp_rewrite("ADVAPI32.dll", "RegOpenKeyExA", a32_regokey);
	}

	if (enabled(EX_SKIPMSG))
		pe->jmp_rewrite("USER32.dll", "MessageBoxA", u32_msgbox);

	dbgprintf(0, "k32_cfile=%08X a32_regckey=%08X\n", pfn_k32_cfile, pfn_a32_regckey);
}


void apihook_cleanup()
{
	::DeleteObject(m_localFont);
	::DeleteObject(m_ansiFont);
}


};	// namespace client
