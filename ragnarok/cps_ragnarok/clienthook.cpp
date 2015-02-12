#include "clienthook.hpp"
#include "lua/lua.hpp"
#include "dllconf.hpp"
#include "peimage.hpp"


extern PEImage* pe;
extern DLLConf* conf;


enum
{
	CANDIDATE_BUFFER=64,
	DENOMINATOR=72,
};


HWND master_wnd = NULL;
bool ask_loading = false;

void *pfn_u32_sw, *pfn_u32_msgbox;
void *pfn_k32_cfile;
void *pfn_a32_regckey, *pfn_a32_regokey;
void *pfn_imm_getclw;
void *pfn_g32_cfont;

////////////////////////////////////////////////////////////////////////////////

// u32_sw

typedef BOOL (WINAPI *U32_SW)(HWND, int);


void set_window_pos()
{
	DWORD swp_flag = SWP_NOOWNERZORDER|SWP_NOZORDER;	// z-orderは変更しない

	int x = conf->get_int("window", "pos_x");
	int y = conf->get_int("window", "pos_y");

	if ((x == -1) && (y == -1))
		swp_flag |= SWP_NOMOVE;

	int cx = conf->get_int("window", "width");
	int cy = conf->get_int("window", "height");

	if ((cx == -1) || (cy == -1))
		swp_flag |= SWP_NOSIZE;
	else
	{
		cx += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
		cy += ::GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + ::GetSystemMetrics(SM_CYCAPTION);
	}

	::SetWindowPos(master_wnd, NULL, x, y, cx, cy, swp_flag);
}


BOOL WINAPI u32_sw(HWND a, int b)
{
	if ((master_wnd==0) && (b==SW_SHOW))
	{
		master_wnd = a;
		set_window_pos();
	}

	return reinterpret_cast<U32_SW>(pfn_u32_sw)(a, b);
}

////////////////////////////////////////////////////////////////////////////////

// u32_msgbox

typedef int (WINAPI *U32_MSGBOX)(HWND, LPCSTR, LPCSTR, UINT);


int WINAPI u32_msgbox(HWND a, LPCSTR b, LPCSTR c, UINT d)
{
	if (c == MB_OK)
		return TRUE;
/*
	if (hangul_msg)
	{
		int len_a = ::MultiByteToWideChar(949, NULL, b, -1, NULL, 0);
		int len_b = ::MultibyteToWideChar(949, NULL, c, -1, NULL, 0);
	}
*/
	return reinterpret_cast<U32_MSGBOX>(pfn_u32_msgbox)(a, b, c, d);
}

////////////////////////////////////////////////////////////////////////////////

// k32_cfile

typedef HANDLE (WINAPI *K32_CFILE_A)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);


HANDLE open_grf(LPCSTR a, DWORD b, DWORD c, LPSECURITY_ATTRIBUTES d, DWORD e, DWORD f, HANDLE g)
{
	char filename[MAX_PATH];
	strcpy(filename, a);		// 初期表示用

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = master_wnd;
	ofn.lpstrFilter = _T("GravityResourceFile(*.grf)\0*.grf\0\0");
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt  = _T("grf");
	ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;

	if (::GetOpenFileName(&ofn) == FALSE)
		strcpy(filename, a);	// cancelなどの場合は元々のfilenameで開く

	return reinterpret_cast<K32_CFILE_A>(pfn_k32_cfile)(a, b, c, d, e, f, g);
}


HANDLE WINAPI k32_cfile(LPCSTR a, DWORD b, DWORD c, LPSECURITY_ATTRIBUTES d, DWORD e, DWORD f, HANDLE g)
{
	// CREATE_ALWAYSはそのまま通す
	if (e == CREATE_ALWAYS)
		return reinterpret_cast<K32_CFILE_A>(pfn_k32_cfile)(a, b, c, d, e, f, g);

	if (ask_loading)
	{
		// abcdef.ghi
		// 0123456789=10
		int ext = strlen(a) - 4;
		if (strcmpi(a+ext, ".grf") == 0)
			return open_grf(a, b, c, d, e, f, g);
	}

	// data\...
	if ((a[0]==0x64) && (a[4]==0x5C))
	{
		wchar_t filename_w[MAX_PATH];
		::MultiByteToWideChar(949, NULL, a, -1, filename_w, MAX_PATH);
		return ::CreateFileW(filename_w, b, c, d, e, f, g);
	}

	return reinterpret_cast<K32_CFILE_A>(pfn_k32_cfile)(a, b, c, d, e, f, g);
}

////////////////////////////////////////////////////////////////////////////////

// a32_regckey/a32_regokey

typedef LONG (WINAPI *A32_REGCKEY_EX_A)(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);


bool _reg_checksubkey(const char* subkeyname)
{
	const char* def = "Software\\Gravity Soft\\Ragnarok\\ShortcutItem\\";
	return strncmp(def, subkeyname, strlen(def))==0;
}

LONG _reg_createsubkey(HKEY a, const char* subkeyname, REGSAM c, LPSECURITY_ATTRIBUTES d, PHKEY e, LPDWORD f, DWORD aid)
{
	size_t length = strlen(subkeyname) + 16;
	char* buf = new char[length];
	
	_sntprintf(buf, length, _T("%s\\%08X"), subkeyname, aid);

	LONG result = reinterpret_cast<A32_REGCKEY_EX_A>(pfn_a32_regckey)(a, buf, 0, NULL, REG_OPTION_NON_VOLATILE, c, d, e, f);

	delete [] buf;

	return result;
}


LONG WINAPI a32_regckey(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i)
{
	DbgPrint(_T("%s"), b);

	DWORD aid_addr = conf->get_addr(ADDR_AID);
	if (::IsBadReadPtr(reinterpret_cast<const void*>(aid_addr), sizeof(DWORD)) == 0)
	{
		if (_reg_checksubkey(b))
			return _reg_createsubkey(a, b, f, g, h, i, *reinterpret_cast<DWORD*>(aid_addr));
	}

	return reinterpret_cast<A32_REGCKEY_EX_A>(pfn_a32_regckey)(a, b, c, d, e, f, g, h, i);
}

LONG WINAPI a32_regokey(HKEY a, LPCSTR b, DWORD c, REGSAM d, PHKEY e)
{
	return a32_regckey(a, b, 0, NULL, REG_OPTION_NON_VOLATILE, d, NULL, e, NULL);
}

////////////////////////////////////////////////////////////////////////////////

// g32_cfont


int get_fontheight(int fontsize)
{
	HDC dc = ::GetDC(master_wnd);
	int val = ::GetDeviceCaps(dc, LOGPIXELSY);
	::ReleaseDC(master_wnd, dc);

	return -MulDiv(fontsize, val, DENOMINATOR);
}


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

	if (i == ANSI_CHARSET)
	{
		if (int fontsize = conf->get_int("font", "ansi_size"))
		{
			int height = get_fontheight(fontsize);

			lf.lfHeight = (height < a) ? height : a;
			lf.lfWeight = conf->get_int("font", "ansi_weight");
			lf.lfQuality = static_cast<BYTE>(conf->get_int("font", "ansi_antialias"));
			lf.lfCharSet = ANSI_CHARSET;

			conf->get_string("font", "ansi_name", lf.lfFaceName, LF_FACESIZE);
		}
	}
	else
	{
		if (int fontsize = conf->get_int("font", "local_size"))
		{
			int height = get_fontheight(fontsize);

			lf.lfHeight = (height < a) ? height : a;
			lf.lfWeight = conf->get_int("font", "local_weight");
			lf.lfQuality = static_cast<BYTE>(conf->get_int("font", "local_antialias"));
			lf.lfCharSet = static_cast<BYTE>(conf->get_int("font", "local_charset"));

			conf->get_string("font", "local_name", lf.lfFaceName, LF_FACESIZE);
		}
	}

	return ::CreateFontIndirect(&lf);
}

////////////////////////////////////////////////////////////////////////////////

// imm_getclw

typedef DWORD (WINAPI *IMM_GETCLW)(HIMC, DWORD, LPCANDIDATELIST, DWORD);


DWORD WINAPI imm_getclw(HIMC a, DWORD b, LPCANDIDATELIST c, DWORD d)
{
	DWORD result = reinterpret_cast<IMM_GETCLW>(pfn_imm_getclw)(a, b, c, d);

	if (d == 0)				// 0の場合はbuffersizeを返すだけ
		return result;

	for (DWORD i=0;i<c->dwCount; i++)
	{
		const wchar_t* ws = reinterpret_cast<wchar_t*>(reinterpret_cast<DWORD*>(c)+c->dwOffset[i]);
		if (wcslen(ws) > CANDIDATE_BUFFER/2)
		{
			c->dwCount = 0;	// 表示させない
			break;
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////


void hook::start()
{
//	pfn_u32_sw = pe->rewriteFF25(_T("USER32.dll"), "ShowWindow", u32_sw);

	bool ask_loading = conf->get_bool("filessystem", "ask_loading_grf");
	if (ask_loading || conf->get_bool("filessystem", "support_unicode"))
		pfn_k32_cfile = pe->rewriteFF25(_T("KERNEL32.dll"), "CreateFileA", k32_cfile);

	bool use_ansi = conf->get_int("font", "ansi_size")!=0;
	bool use_local = conf->get_int("font", "local_size")!=0;
	if (use_ansi || use_local)
		pfn_g32_cfont = pe->rewriteFF25(_T("GDI32.dll"), "CreateFontA", g32_cfont);

	if (conf->get_bool("shortcut", "save_extended"))
	{
		pfn_a32_regckey = pe->rewriteFF25(_T("ADVAPI32.dll"), "RegCreateKeyExA", a32_regckey);
		pfn_a32_regokey = pe->rewriteFF25(_T("ADVAPI32.dll"), "RegOpenKeyExA", a32_regokey);
	}

	bool msg_hangul = conf->get_bool("extention", "shown_hangul_msg");
	if (msg_hangul || conf->get_bool("extention", "skip_error_msg"))
		pfn_u32_msgbox = pe->rewriteFF25(_T("USER32.dll"), "MessageBoxA", u32_msgbox);

	if (conf->get_bool("extention", "fixed_ime"))
		pfn_imm_getclw = pe->rewriteFF25("IMM32.dll", "ImmGetCandidateListW", imm_getclw);
}
