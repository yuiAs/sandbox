#if defined(WIN32)
#include <windows.h>
#endif
#include "debug.h"
#include "fileOut.hpp"


FileOut* g_DebugOut = NULL;
int g_DebugLevel = DBGREL;

////////////////////////////////////////////////////////////////////////////////


void DbgPrintW(int Level, wchar_t* Format, ...)
{
	if (Level > g_DebugLevel)
		return;

	va_list va;
	va_start(va, Format);

	int Request = _vscwprintf(Format, va);
	if (Request != -1)
	{
		wchar_t* Buffer = new wchar_t[Request+1];

		int Result = _vsnwprintf(Buffer, Request+1, Format, va);
		if (Result > 0)
			g_DebugOut->Write(Buffer, Result<<1);

		delete [] Buffer;
	}

	va_end(va);
}

////////////////////////////////////////////////////////////////////////////////
#if defined(WIN32) && (NTDDI_VERSION>=NTDDI_WINXP)
#include <wincrypt.h>


void _ReleaseB2H(PVOID Address)
{
	VirtualFree(Address, 0, MEM_RELEASE);
}


ULONG _Bin2Hex(CONST PBYTE Binary, ULONG Length, PWSTR* PtrBuffer)
{
	BOOL Result = FALSE;
	ULONG Request = 0;

	Result = CryptBinaryToStringW(Binary, Length, CRYPT_STRING_HEXASCII, NULL, &Request);
	if (Result == FALSE)
		return -1;

	PWSTR Buffer = reinterpret_cast<PWSTR>(VirtualAlloc(NULL, Request<<1, MEM_COMMIT, PAGE_READWRITE));
	if (Buffer == NULL)
		return -1;

	Result = CryptBinaryToStringW(Binary, Length, CRYPT_STRING_HEXASCII, Buffer, &Request);
	if (Result == FALSE)
		_ReleaseB2H(Buffer);
	else
	{
		*PtrBuffer = Buffer;
		return Request;
	}

	return -1;
}


void DbgHexPrintW(CONST PBYTE Data, ULONG Length)
{
	if (DBGHEX > g_DebugLevel)
		return;

	PWSTR Buffer = NULL;

	ULONG Result = _Bin2Hex(Data, Length, &Buffer);
	if (Result != -1)
	{
		g_DebugOut->Write(Buffer, Result<<1);
		_ReleaseB2H(Buffer);
	}
}

#endif	// #if defined(WIN32) && (NTDDI_VERSION>=NTDDI_WINXP)
////////////////////////////////////////////////////////////////////////////////


void InitDebug(int DebugLevel)
{
	if (g_DebugOut == NULL)
	{
		g_DebugOut = new FileOut(_CRT_WIDE("debug.txt"));

		if (g_DebugOut->Size() == 0)
		{
			unsigned char Bom[] = { 0xFF, 0xFE };
			g_DebugOut->Write(Bom, sizeof(Bom));
		}
	}

	g_DebugLevel = DebugLevel;
}


void CloseDebug()
{
	if (g_DebugOut)
	{
		delete g_DebugOut;
		g_DebugOut = NULL;
	}
}
