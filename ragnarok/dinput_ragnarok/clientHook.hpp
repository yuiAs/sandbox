#ifndef CLIENT_HOOK_HPP
#define CLIENT_HOOK_HPP


#include <windows.h>
#include <tchar.h>
#include "Ragnarok.hpp"


class CClientHook
{
	enum
	{
		MAX_CANDIDATE_BUFSIZE=32,	// multibyte32word(64byte)
		DENOMINATOR=72,
	};

private:

	static CClientHook* m_this;

	CRagnarok* m_core;

	void *m_pfn_sw;
	void *m_pfn_cfont, *m_pfn_cfile, *m_pfn_msgbox, *m_pfn_immgcl, *m_pfn_regckey, *m_pfn_regokey;

	bool ask_loading;

	HWND m_wnd;

public:

	CClientHook();
	~CClientHook() {}

private:

	void initialize();

	void setWindowPos(HWND wnd);
	HANDLE openGrfFile(const TCHAR* filename, DWORD b, DWORD c, LPSECURITY_ATTRIBUTES d, DWORD creation, DWORD f, HANDLE g);
	int getFontHeight(int fontsize);
	bool checkSubKey(const TCHAR* subKeyname);
	LONG createSubKey(HKEY key, const TCHAR* subKeyname, REGSAM c, LPSECURITY_ATTRIBUTES d, PHKEY e, LPDWORD f, DWORD dwAID);

public:

	HWND getWnd() const { return m_wnd; }

	// api-hook

private:

	BOOL u32_sw(HWND hWnd, int nCmdShow);
	int u32_msgbox(HWND hWnd, LPCSTR lpText, LPCTSTR lpCaption, UINT uType);
	HANDLE k32_cfile(LPCSTR lpFileName, DWORD b, DWORD c, LPSECURITY_ATTRIBUTES d, DWORD dwCreateDisposition, DWORD f, HANDLE g);
	HFONT g32_cfont(int a, int b, int c, int d, int e, DWORD f, DWORD g, DWORD h, DWORD i, DWORD j, DWORD k, DWORD l, DWORD m, LPCSTR n);
	DWORD imm_immgcl(HIMC hIMC, DWORD dwIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen);
	LONG a32_regckey(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i);
	//LONG a32_regokey(HKEY a, LPCSTR b, DWORD c, REGSAM d, PHKEY e);

public:

	static BOOL WINAPI _ShowWindow(HWND a, int b)
		{ return CClientHook::m_this->u32_sw(a, b); }
	static int WINAPI _MessageBoxA(HWND a, LPCSTR b, LPCTSTR c, UINT d)
		{ return CClientHook::m_this->u32_msgbox(a, b, c, d); }
	static HANDLE WINAPI _CreateFileA(LPCSTR a, DWORD b, DWORD c, LPSECURITY_ATTRIBUTES d, DWORD e, DWORD f, HANDLE g)
		{ return CClientHook::m_this->k32_cfile(a, b, c, d, e, f, g); }
	static HFONT WINAPI _CreateFontA(int a, int b, int c, int d, int e, DWORD f, DWORD g, DWORD h, DWORD i, DWORD j, DWORD k, DWORD l, DWORD m, LPCSTR n)
		{ return CClientHook::m_this->g32_cfont(a, b, c, d, e, f, g, h, i, j, k, k, m, n); }
	static DWORD WINAPI _ImmGetCandidateListW(HIMC a, DWORD b, LPCANDIDATELIST c, DWORD d)
		{ return CClientHook::m_this->imm_immgcl(a, b, c, d); }
	static LONG WINAPI _RegCreateKeyExA(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i)
		{ return CClientHook::m_this->a32_regckey(a, b, c, d, e, f, g, h, i); }
	static LONG WINAPI _RegOpenKeyExA(HKEY a, LPCSTR b, DWORD c, REGSAM d, PHKEY e)
		{ return CClientHook::m_this->a32_regckey(a, b, 0, NULL, REG_OPTION_NON_VOLATILE, d, NULL, e, NULL); }
};


#endif	// #ifndef CLIENT_HOOK_HPP
