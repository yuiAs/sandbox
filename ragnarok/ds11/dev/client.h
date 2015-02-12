#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "pe.h"
#include <map>
#include "client_enum.h"


class Client : public PEOp
{
	DWORD m_address[ADDRESS_MAX];
	std::map<DWORD, DWORD> m_importTable;

public:

	Client();
	~Client();

public:

	void init();

	void analysis();
	void hook();

public:

	DWORD GetAddress(enum ADDRESS type) { return m_address[type]; }

public:

	void unprotect_sakray();
	void* apihook(LPCTSTR module, LPCSTR name, void* func);

	void WritePrivateProfileHex(LPCTSTR lpAppName, LPCTSTR lpKeyName, DWORD val, LPCTSTR lpFileName); 

public:

	HRESULT clprintf(COLORREF color, const char* fmt, ...);
	HRESULT GetMsgString(int num, char** buf);
	HRESULT SetCondition(unsigned short _type, unsigned char _flag);
	HRESULT SetCondition(unsigned char* buf);

private:

	HRESULT clprintf(const char* buf, COLORREF color);

private:

	bool GetInstruction(DWORD address, DWORD range, DWORD* count, x86_insn_t* insn);
	inline DWORD GetCallAddress(DWORD address, x86_insn_t* insn);

	DWORD FindPushString(const char* text);

private:

	void search_pattern1(DWORD addres, DWORD range, DWORD val, enum ADDRESS type);
	void search_APIJmpTable(DWORD range);

	void search_netbase(DWORD range);
	void search_send(DWORD range);
	void search_recv(DWORD range);
	void search_gamebase(DWORD range);
	void search_drawbase(DWORD range);
	void search_aid(DWORD range);
	void search_charname(DWORD range1, DWORD range2);
	void search_msgTable(DWORD range);
	void search_zoneParser(DWORD range);

private:

	static int WINAPI dummy_MessageBoxA(HWND a, LPCSTR b, LPCSTR c, UINT d);
	int api_MessageBoxA(HWND a, LPCSTR b, LPCSTR c, UINT d);

	static LONG WINAPI dummy_RegCreateKeyExA(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i);
	static LONG WINAPI dummy_RegOpenKeyExA(HKEY a, LPCSTR b, DWORD c, REGSAM d, PHKEY e);
	LONG api_RegCreateKeyExA(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i);
	LONG api_RegOpenKeyExA(HKEY a, LPCSTR b, DWORD c, REGSAM d, PHKEY e);

	bool is_shortcutitem(const char* subkey);
	LONG api_RegCreateKeyExW(HKEY a, LPCSTR b, DWORD c, LPSTR d, DWORD e, REGSAM f, LPSECURITY_ATTRIBUTES g, PHKEY h, LPDWORD i);
};


#endif	// #ifndef CLIENT_H_INCLUDED
