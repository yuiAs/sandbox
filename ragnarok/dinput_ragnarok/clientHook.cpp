#include "clientHook.hpp"
#include <commdlg.h>
#include <shlwapi.h>
#include "apiHook.hpp"


CClientHook* CClientHook::m_this = 0;

////////////////////////////////////////////////////////////////////////////////


CClientHook::CClientHook() : ask_loading(false), m_wnd(0)
{
	m_this = this;

	extern CRagnarok* core;
	m_core = core;

	initialize();
}


void CClientHook::initialize()
{
	APIHook* api = new APIHook;

	m_pfn_sw = api->HookAPICall(_T("USER32.dll"), _T("ShowWindow"), _ShowWindow);

	if (::GetPrivateProfileInt(_T("system"), _T("unicode_fs"), 0, _T("./dinput.ini")))
	{
		if (::GetPrivateProfileInt(_T("system"), _T("ask_loading_grf"), 0, _T("./dinput.ini")))
			ask_loading = true;

		::SetThreadLocale(0x0412);	// 韓国語エンコードなファイル読み込めるように
		m_pfn_cfile = api->HookAPICall(_T("KERNEL32.dll"), _T("CreateFileA"), _CreateFileA);
	}

	if (::GetPrivateProfileInt(_T("font"), _T("enable"), 0, _T("./dinput.ini")))
	{
		m_pfn_cfont = api->HookAPICall(_T("GDI32.dll"), _T("CreateFontA"), _CreateFontA);
	}

	if (::GetPrivateProfileInt(_T("system"), _T("skip_errormsg"), 0, _T("./dinput.ini")))
	{
		m_pfn_msgbox = api->HookAPICall(_T("USER32.dll"), _T("MessageBoxA"), _MessageBoxA);
	}

	if (::GetPrivateProfileInt(_T("system"), _T("fix_ime"), 0, _T("./dinput.ini")))
	{
		m_pfn_immgcl = api->HookAPICall("IMM32.dll", "ImmGetCandidateListW", _ImmGetCandidateListW);
	}

	if (::GetPrivateProfileInt(_T("shortcut"), _T("extended_reg"), 0, _T("./dinput.ini")))
	{
		m_pfn_regckey = api->HookAPICall(_T("Advapi32.dll"), _T("RegCreateKeyExA"), _RegCreateKeyExA);
		m_pfn_regokey = api->HookAPICall(_T("Advapi32.dll"), _T("RegOpenKeyExA"), _RegOpenKeyExA);
	}

	delete api;
}

////////////////////////////////////////////////////////////////////////////////

// 使いまわす

typedef HANDLE (WINAPI *LPCREATEFILEA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef LONG (WINAPI *LPREGCREATEKEYEXA)(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);

// support u32_sw

void CClientHook::setWindowPos(HWND wnd)
{
	DWORD swp_flag = SWP_NOOWNERZORDER|SWP_NOZORDER;

	// Window Potision

	int x = static_cast<int>(::GetPrivateProfileInt(_T("window"), _T("pos_x"), -1, _T("./dinput.ini")));
	int y = static_cast<int>(::GetPrivateProfileInt(_T("window"), _T("pos_y"), -1, _T("./dinput.ini")));

	if ((x == -1) && (y == -1))
		swp_flag |= SWP_NOMOVE;

	// Window Size

	int cx = static_cast<int>(::GetPrivateProfileInt(_T("window"), _T("width"), -1, _T("./dinput.ini")));
	int cy = static_cast<int>(::GetPrivateProfileInt(_T("window"), _T("height"), -1, _T("./dinput.ini")));

	if ((cx == -1) || (cy == -1))
		swp_flag |= SWP_NOSIZE;
	else
	{
		cx += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
		cy += ::GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + ::GetSystemMetrics(SM_CYCAPTION);
	}

	m_wnd = wnd;
	::SetWindowPos(wnd, NULL, x, y, cx, cy, swp_flag);
}

// supprt k32_cfile

HANDLE CClientHook::openGrfFile(const TCHAR* filename, DWORD b, DWORD c, LPSECURITY_ATTRIBUTES d, DWORD creation, DWORD f, HANDLE g)
{
	TCHAR buffer[MAX_PATH];
	::lstrcpy(buffer, filename);

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_wnd;
	ofn.lpstrFilter = _T("GravityResourceFile(*.grf)\0*.grf\0\0");
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt  = _T("grf");
	ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;

	if (::GetOpenFileName(&ofn) == FALSE)
		::lstrcpy(buffer, filename);

	return reinterpret_cast<LPCREATEFILEA>(m_pfn_cfile)(buffer, b, c, d, creation, f, g);
}

// support g32_cfont

int CClientHook::getFontHeight(int fontsize)
{
	HDC dc = ::GetDC(m_wnd);
	int val = ::GetDeviceCaps(dc, LOGPIXELSY);
	::ReleaseDC(m_wnd, dc);

	return -MulDiv(fontsize, val, DENOMINATOR);
}

// support a32_regckey

bool CClientHook::checkSubKey(const TCHAR* subKeyname)
{
	const TCHAR* defKeyname = _T("Software\\Gravity Soft\\Ragnarok\\ShortcutItem\\");
	return _tcsncmp(defKeyname, subKeyname, ::lstrlen(defKeyname))==0;
}

LONG CClientHook::createSubKey(HKEY a, const TCHAR* subKeyname, REGSAM c, LPSECURITY_ATTRIBUTES d, PHKEY e, LPDWORD f, DWORD dwAID)
{
	//TCHAR* newKeyname = new TCHAR[::lstrlen(subKeyname)+16];
	int length = ::lstrlen(subKeyname) + 16;	
	TCHAR* newKeyname = new TCHAR[length];
	_sntprintf(newKeyname, length, _T("%s\\%08X"), subKeyname, dwAID);

	m_core->output(_T("CClientHook::createSubKey=%s"), newKeyname);

	LONG result = reinterpret_cast<LPREGCREATEKEYEXA>(m_pfn_regckey)(a, newKeyname, 0, NULL, REG_OPTION_NON_VOLATILE, c, d, e, f);

	delete [] newKeyname;

	return result;
}

////////////////////////////////////////////////////////////////////////////////


BOOL CClientHook::u32_sw(HWND hWnd, int nCmdShow)
{
	if ((m_wnd == 0) && (nCmdShow == SW_SHOW))
		setWindowPos(hWnd);

	typedef BOOL (WINAPI *LPSHOWWINDOW)(HWND, int);
	return reinterpret_cast<LPSHOWWINDOW>(m_pfn_sw)(hWnd, nCmdShow);
}

////////////////////////////////////////////////////////////////////////////////


int CClientHook::u32_msgbox(HWND hWnd, LPCSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	if (uType == MB_OK)
		return IDOK;

	typedef int (WINAPI *LPMESSAGEBOXA)(HWND, LPCSTR, LPCTSTR, UINT);
	return reinterpret_cast<LPMESSAGEBOXA>(m_pfn_msgbox)(hWnd, lpText, lpCaption, uType);
}

////////////////////////////////////////////////////////////////////////////////


HANDLE CClientHook::k32_cfile(LPCSTR lpFileName, DWORD b, DWORD c, LPSECURITY_ATTRIBUTES d, DWORD dwCreateDisposition, DWORD f, HANDLE g)
{
	// CREATE_ALWAYS (0x00000002)はそのまま
	if (dwCreateDisposition == CREATE_ALWAYS)
		return reinterpret_cast<LPCREATEFILEA>(m_pfn_cfile)(lpFileName, b, c, d, dwCreateDisposition, f, g);

	if (ask_loading && (lstrcmpi(_T(".grf"), ::PathFindExtension(lpFileName)) == 0))
		return openGrfFile(lpFileName, b, c, d, dwCreateDisposition, f, g);

	// "d***\"の場合はunicodeにしてから
	if ((lpFileName[0] == 0x64) && (lpFileName[4] == 0x5C))
	{
		WCHAR lpwFileName[MAX_PATH];
		::MultiByteToWideChar(949, NULL, lpFileName, -1, lpwFileName, lstrlen(lpFileName)+1);

		return ::CreateFileW(lpwFileName, b, c, d, dwCreateDisposition, f, g);
	}

	return reinterpret_cast<LPCREATEFILEA>(m_pfn_cfile)(lpFileName, b, c, d, dwCreateDisposition, f, g);
}

////////////////////////////////////////////////////////////////////////////////


HFONT CClientHook::g32_cfont(int a, int b, int c, int d, int e, DWORD f, DWORD g, DWORD h, DWORD i, DWORD j, DWORD k, DWORD l, DWORD m, LPCSTR n)
{
	LOGFONT lf;
	::ZeroMemory(&lf, sizeof(LOGFONT));

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
	::CopyMemory(lf.lfFaceName, n, LF_FACESIZE);


	if (i == ANSI_CHARSET)
	{
		if (int fontSize = static_cast<int>(::GetPrivateProfileInt(_T("font"), _T("ansi_fontsize"), 0, _T("./dinput.ini"))))
		{
			int fontHeight = getFontHeight(fontSize);

			lf.lfHeight = (fontHeight < a) ? fontHeight : a;
			lf.lfWeight = ::GetPrivateProfileInt(_T("font"), _T("ansi_fontweight"), e, _T("./dinput.ini"));
			lf.lfQuality = static_cast<BYTE>(::GetPrivateProfileInt(_T("font"), _T("ansi_antialias"), l, _T("./dinput.ini")));
			lf.lfCharSet = ANSI_CHARSET;

			::GetPrivateProfileString(_T("font"), _T("ansi_fontname"), n, lf.lfFaceName, LF_FACESIZE, _T("./dinput.ini"));
		}
	}
	else
	{
		if (int fontSize = static_cast<int>(::GetPrivateProfileInt(_T("font"), _T("local_fontsize"), 0, _T("./dinput.ini"))))
		{
			int fontHeight = getFontHeight(fontSize);

			lf.lfHeight = (fontHeight < a) ? fontHeight : a;
			lf.lfWeight = ::GetPrivateProfileInt(_T("font"), _T("local_fontweight"), e, _T("./dinput.ini"));
			lf.lfQuality = static_cast<BYTE>(::GetPrivateProfileInt(_T("font"), _T("local_antialias"), l, _T("./dinput.ini")));
			lf.lfCharSet = static_cast<BYTE>(::GetPrivateProfileInt(_T("font"), _T("local_charset"), i, _T("./dinput.ini")));

			::GetPrivateProfileString(_T("font"), _T("local_fontname"), n, lf.lfFaceName, LF_FACESIZE, _T("./dinput.ini"));
		}
	}


	return ::CreateFontIndirect(&lf);
}

////////////////////////////////////////////////////////////////////////////////


DWORD CClientHook::imm_immgcl(HIMC hIMC, DWORD dwIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen)
{
	// originalを呼び出す
	// return valueはCANDIDATALISTのbyte数
	typedef DWORD (WINAPI *LPIMMGETCANDIDATELISTW)(HIMC, DWORD, LPCANDIDATELIST, DWORD);
	DWORD dwResult = reinterpret_cast<LPIMMGETCANDIDATELISTW>(m_pfn_immgcl)(hIMC, dwIndex, lpCandList, dwBufLen);

	// dwBufLen==0の場合、dwResultはcandidate listを受け取るのに必要なbyte数
	if (dwBufLen != 0)
	{
		for (DWORD i=0; i<lpCandList->dwCount; i++)
		{
			LPCWSTR lpwString = reinterpret_cast<LPCWSTR>(reinterpret_cast<LPCSTR>(lpCandList)+lpCandList->dwOffset[i]);
			if (lstrlenW(lpwString) > MAX_CANDIDATE_BUFSIZE)
				lpCandList->dwCount = 0;
				break;
		}		
	}

	return dwResult;
}

////////////////////////////////////////////////////////////////////////////////


LONG CClientHook::a32_regckey(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i)
{
	if (DWORD dwAID = m_core->getActorVal(ACTOR_SID))
	{
		if (checkSubKey(b))
			return createSubKey(a, b, f, g, h, i, dwAID);
	}

	return reinterpret_cast<LPREGCREATEKEYEXA>(m_pfn_regckey)(a, b, c, d, e, f, g, h, i);
}
