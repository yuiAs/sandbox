#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "WinAdvApi.h"

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   CreateAnsiStringW
//  Notes   :   WideCharをSourceに内部でUNICODE_STRING->ANSI_STRING変換を行う。
//              返されたANSI_STRINGはRtlFreeAnsiStringで開放しなければならない。
//------------------------------------------------------------------------------
NTSTATUS NTAPI CreateAnsiStringW(PANSI_STRING DestinationString, PWSTR SourceString)
{
    UNICODE_STRING UnicodeString;
    RtlInitUnicodeString(&UnicodeString, SourceString);
    return RtlUnicodeStringToAnsiString(DestinationString, &UnicodeString, TRUE);
}

//------------------------------------------------------------------------------
//  Function:   CreateUnicodeStringA
//  Notes   :   CharをSourceに内部でANSI_STRING->UNICODE_STRING変換を行う。
//              返されたUNICODE_STRINGはRtlFreeUnicodeStringで開放しなければならない。
//------------------------------------------------------------------------------
NTSTATUS NTAPI CreateUnicodeStringA(PUNICODE_STRING DestinationString, PSTR SourceString)
{
    ANSI_STRING AnsiString;
    RtlInitAnsiString(&AnsiString, SourceString);
    return RtlAnsiStringToUnicodeString(DestinationString, &AnsiString, TRUE);
}

////////////////////////////////////////////////////////////////////////////////

#if (defined(DBG)||defined(_CHKBUILD))

#include <wchar.h>
#include <stdio.h>

// C4996: This function or variable may be unsafe.
#pragma warning(disable: 4996)

//------------------------------------------------------------------------------
//  Function:   DbgPrintW
//  Notes   :   DBG未定義時は__noopで無効化。
//------------------------------------------------------------------------------
VOID DbgPrintW(CONST PWSTR Format, ...)
{
    va_list va;
    va_start(va, Format);

    int Request = _vscwprintf(Format, va);
    if (Request != -1)
    {
        PWSTR Buffer = reinterpret_cast<PWSTR>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR)*(Request+1)));
        // 面倒なので確実に確保できてることを期待

        int Result = _vsnwprintf(Buffer, Request, Format, va);
        if (Result > 0)
            OutputDebugStringW(Buffer);

        HeapFree(GetProcessHeap(), 0, Buffer);
    }

    va_end(va);
}

//------------------------------------------------------------------------------
//  Function:   DbgPrintA
//  Notes   :   DBG未定義時は__noopで無効化。
//------------------------------------------------------------------------------
VOID DbgPrintA(CONST PSTR Format, ...)
{
    va_list va;
    va_start(va, Format);

    int Request = _vscprintf(Format, va);
    if (Request != -1)
    {
        PSTR Buffer = reinterpret_cast<PSTR>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Request+1));
        // 面倒なので確実に確保できてることを期待

        int Result = _vsnprintf(Buffer, Request, Format, va);
        if (Result > 0)
        {
#if 0
            OutputDebugStringA(Buffer);
#else
            UNICODE_STRING UnicodeString;
            CreateUnicodeStringA(&UnicodeString, Buffer);
            OutputDebugStringW(UnicodeString.Buffer);
            RtlFreeUnicodeString(&UnicodeString);
#endif
        }

        HeapFree(GetProcessHeap(), 0, Buffer);
    }

    va_end(va);
}

#endif  // DBG

//------------------------------------------------------------------------------
//  Function:   OutputExceptionInfo
//  Notes   :   DBG未定義時はEXCEPTION_EXECUTE_HANDLERを返すのみ。
//------------------------------------------------------------------------------
ULONG WINAPI OutputExceptionInfo(EXCEPTION_POINTERS* e)
{
#if (defined(DBG)||defined(_CHKBUILD))
    DbgPrintW(_L("/********** EXCEPTION **********/"));

    PEXCEPTION_RECORD r = e->ExceptionRecord;
    DbgPrintW(_L("ADDR=%08X\n"), r->ExceptionAddress);
    DbgPrintW(_L("CODE=%08X\n"), r->ExceptionCode);

    PCONTEXT c = e->ContextRecord;
    DbgPrintW(_L("EAX=%08X\n"), c->Eax);
    DbgPrintW(_L("EBX=%08X\n"), c->Ebx);
    DbgPrintW(_L("ECX=%08X\n"), c->Ecx);
    DbgPrintW(_L("EDX=%08X\n"), c->Edx);
    DbgPrintW(_L("ESI=%08X\n"), c->Esi);
    DbgPrintW(_L("EDI=%08X\n"), c->Edi);
    DbgPrintW(_L("ESP=%08X\n"), c->Esp);
    DbgPrintW(_L("EBP=%08X\n"), c->Ebp);
    DbgPrintW(_L("EIP=%08X\n"), c->Eip);
#endif

    return EXCEPTION_EXECUTE_HANDLER;
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   CreatePrivateHeap
//  Notes   :   PrivateHeapを作成しLFHを有効にする。
//              作成できないときはProcessHeapを返す。
//------------------------------------------------------------------------------
HANDLE WINAPI CreatePrivateHeap(ULONG Options, SIZE_T InitialSize, SIZE_T MaxiumSize)
{
    HANDLE Handle = HeapCreate(Options, InitialSize, MaxiumSize);
    if (Handle == NULL)
        Handle = GetProcessHeap();
    else
    {
        ULONG Value = 2;
        HeapSetInformation(Handle, HeapCompatibilityInformation, &Value, sizeof(ULONG));
    }

    return Handle;
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   GetLocalTimeAsFileTime
//  Notes   :   LocalTimeをFILETIME形式で取得する。
//------------------------------------------------------------------------------
VOID WINAPI GetLocalTimeAsFileTime(FILETIME* LocalTimeAsFileTime)
{
    FILETIME FileTime;
    GetSystemTimeAsFileTime(&FileTime);
    FileTimeToLocalFileTime(&FileTime, LocalTimeAsFileTime);
}

//------------------------------------------------------------------------------
//  Function:   SecondToNanoSecond
//  Notes   :   secをnsecに変換する。
//------------------------------------------------------------------------------
ULONGLONG WINAPI SecondToNanoSecond(ULONG Second)
{
    return UInt32x32To64(1000*1000*10, Second);
}

//------------------------------------------------------------------------------
//  Function:   SystemTimeToFileTimeAdd
//  Notes   :   SystemTimeToFileTime拡張
//------------------------------------------------------------------------------
VOID WINAPI SystemTimeToFileTimeAdd(CONST SYSTEMTIME* SystemTime, LONGLONG Add, FILETIME* FileTime)
{
    SystemTimeToFileTime(SystemTime, FileTime);
    ULONGLONG Value = *reinterpret_cast<ULONGLONG*>(FileTime) + Add;
    RtlCopyMemory(FileTime, &Value, sizeof(ULONGLONG));
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   CreateHexDumpW
//  Notes   :   HexDump+ASCII出力をする。
//              返されたBufferはReleaseHexDumpで開放しなければならない。
//              現在のところ改行は他との兼ね合いでLFのみ。
//              Flagは予約で0。
//------------------------------------------------------------------------------
PVOID WINAPI CreateHexDumpW(CONST PUCHAR Data, ULONG Length, ULONG Flags)
{
    ULONG Request = 0;
    PVOID Buffer = NULL;
    BOOL Result = FALSE;

#ifdef _ENABLE_WINCRYPTAPI_
    // バッファサイズ取得
    Result = CryptBinaryToStringW(Data, Length, CRYPT_STRING_HEXASCII|CRYPT_STRING_NOCR, NULL, &Request);
    if (Result == FALSE)
        return NULL;

    // バッファ確保
    Buffer = VirtualAllocEx(GetCurrentProcess(), NULL, Request<<1, MEM_COMMIT, PAGE_READWRITE);
    if (Buffer == NULL)
        return NULL;

    // Dump
    Result = CryptBinaryToStringW(Data, Length, CRYPT_STRING_HEXASCII|CRYPT_STRING_NOCR, reinterpret_cast<PWSTR>(Buffer), &Request);
    if (Result == FALSE)
    {
        VirtualFreeEx(GetCurrentProcess(), Buffer, 0, MEM_RELEASE);
        return NULL;
    }
#endif

    return Buffer;
}

//------------------------------------------------------------------------------
//  Function:   ReleaseHexDump
//  Notes   :   
//------------------------------------------------------------------------------
VOID WINAPI ReleaseHexDump(PVOID Buffer)
{
    if (Buffer)
        VirtualFreeEx(GetCurrentProcess(), Buffer, 0, MEM_RELEASE);
}
