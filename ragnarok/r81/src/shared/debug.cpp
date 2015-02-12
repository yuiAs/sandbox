//! @file   debug.cpp
#pragma comment(lib, "strsafe.lib")
#pragma comment(lib, "CRYPT32.LIB")

#include <windows.h>
#include <strsafe.h>
#include <wincrypt.h>
#include "debug.h"


//! Variables
UCHAR dbgBuf[_USRDBGBUF];


//! Output debug string. (Formatted)
//! @note   max characters = _USRDBGBUF
void UsrDbgPringA(const char* FormatString, ...)
{
    va_list varList;
    va_start(varList, FormatString);

    STRSAFE_LPSTR buf = reinterpret_cast<STRSAFE_LPSTR>(dbgBuf);

    HRESULT hr = StringCbVPrintfA(buf, _USRDBGBUF, FormatString, varList);
    if (SUCCEEDED(hr))
        OutputDebugStringA(buf);

    va_end(varList);
}

//! Output debug string. (Formatted)
//! @note   max characters = _USRDBGBUF/2
void UsrDbgPrintW(const wchar_t* FormatString, ...)
{
    va_list varList;
    va_start(varList, FormatString);

    wchar_t* buf = reinterpret_cast<wchar_t*>(dbgBuf);

    HRESULT hr = StringCbVPrintfW(buf, _USRDBGBUF, FormatString, varList);
    if (SUCCEEDED(hr))
        OutputDebugStringW(buf);

    va_end(varList);
}

//! -
ULONG DbgExceptionInfo(EXCEPTION_POINTERS* e)
{
    //DbgPrintW(L"*** EXCEPTION ***");
    OutputDebugStringW(L"*** EXCEPTION ***");

    PEXCEPTION_RECORD r = e->ExceptionRecord;
    DbgPrintW(L"ADDR=%08X\n", r->ExceptionAddress);
    DbgPrintW(L"CODE=%08X\n", r->ExceptionCode);

    PCONTEXT c = e->ContextRecord;
    DbgPrintW(L"EAX=%08X\n", c->Eax);
    DbgPrintW(L"EBX=%08X\n", c->Ebx);
    DbgPrintW(L"ECX=%08X\n", c->Ecx);
    DbgPrintW(L"EDX=%08X\n", c->Edx);
    DbgPrintW(L"EBP=%08X\n", c->Ebp);
    DbgPrintW(L"ESP=%08X\n", c->Esp);
    DbgPrintW(L"EDI=%08X\n", c->Edi);
    DbgPrintW(L"ESI=%08X\n", c->Esi);
    DbgPrintW(L"EIP=%08X\n", c->Eip);

    return EXCEPTION_EXECUTE_HANDLER;
}

//! -
void UsrDbgDump(const PUCHAR Binary, SIZE_T BinaryLength, ULONG Flags)
{
    if (PVOID dump = ConvertBinaryToHexDump(Binary, BinaryLength, 0))
    {
        OutputDebugString(reinterpret_cast<wchar_t*>(dump));
        VirtualFreeEx(GetCurrentProcess(), dump, 0, MEM_RELEASE);
    }
}

//! Convert array of bytes into Hexadecimal string with ASCII character.
//! @param  Binary          
//! @param  BinaryLength    
//! @param  Flags           (RESERVED)
//! @return wchar_t*
//! @attention  When you no longer need the buffer, call VirtualFree function to delete it.
PVOID ConvertBinaryToHexDump(const PUCHAR Binary, SIZE_T BinaryLength, ULONG Flags)
{
    Flags = 0;
    Flags |= CRYPT_STRING_HEXASCII|CRYPT_STRING_NOCR;

    ULONG reqLen = 0;
    PVOID buf = NULL;

    BOOL result = CryptBinaryToStringW(Binary, BinaryLength, Flags, NULL, &reqLen);
    if (result == FALSE)
        return NULL;

    buf = VirtualAllocEx(GetCurrentProcess(), NULL, reqLen<<1, MEM_COMMIT, PAGE_READWRITE);
    if (buf == NULL)
        return NULL;

    result = CryptBinaryToStringW(Binary, BinaryLength, Flags, reinterpret_cast<wchar_t*>(buf), &reqLen);
    if (result == FALSE)
    {
        VirtualFreeEx(GetCurrentProcess(), buf, 0, MEM_RELEASE);
        return NULL;
    }

    return buf;
}
