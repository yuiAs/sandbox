//--------------------------------------------------------------------------------------
// File: winumadv.cpp
//--------------------------------------------------------------------------------------
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "winumadv.h"

#if defined(DBG) || defined(_DEBUG)
#include <strsafe.h>
#pragma comment(lib, "strsafe.lib")
#endif

#ifdef _WINCRYPT_ENABLED
#include <wincrypt.h>
#pragma comment(lib, "CRYPT32.LIB")
#endif

#ifdef _GDIPLUS_ENABLED
#include <ole2.h>
#include <gdiplus.h>
#pragma comment(lib, "GdiPlus.lib")
#endif

#ifndef DBGPRINT_MAXCB
#define DBGPRINT_MAXCB 512
#endif


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
SIZE_T DbgPrintMaxCB = DBGPRINT_MAXCB;

//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifdef _WINSDK_BUILD
#define ASSOCIATE_NATIVEAPI(api) (FARPROC&)Um##api = GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), #api)
#endif  // _WINSDK_BUILD

//--------------------------------------------------------------------------------------
// WideChar から ANSI_STRING を作成する.
// 内部でバッファを確保するため、作成した ANSI_STRING は RtlFreeAnsiString で開放する.
//--------------------------------------------------------------------------------------
NTSTATUS NTAPI CreateAnsiStringW(PANSI_STRING DestinationString, PCWSTR SourceString)
{
    UNICODE_STRING UnicodeString;
    RtlInitUnicodeString(&UnicodeString, SourceString);
    return RtlUnicodeStringToAnsiString(DestinationString, &UnicodeString, TRUE);
}

//--------------------------------------------------------------------------------------
// Char から UNICODE_STRING を作成する.
// 内部でバッファを確保するため、作成した UNICODE_STRING は RtlFreeUnicodeString で開放する.
//--------------------------------------------------------------------------------------
NTSTATUS NTAPI CreateUnicodeStringA(PUNICODE_STRING DestinationString, PCSZ SourceString)
{
    // RtlCreateUnicodeStringFromAsciiz
    ANSI_STRING AnsiString;
    RtlInitAnsiString(&AnsiString, SourceString);
    return RtlAnsiStringToUnicodeString(DestinationString, &AnsiString, TRUE);
}

#if defined(DBG) || defined(_DEBUG)

//--------------------------------------------------------------------------------------
VOID __cdecl DbgPrintW(PCWSTR FormatString, ...)
{
    va_list ArgList;
    va_start(ArgList, FormatString);

    PWSTR Buffer = reinterpret_cast<PWSTR>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DbgPrintMaxCB));
    if (Buffer != NULL)
    {
        HRESULT hr = StringCbVPrintfW(Buffer, DbgPrintMaxCB, FormatString, ArgList);
        if (SUCCEEDED(hr))
            OutputDebugStringW(Buffer);

        HeapFree(GetProcessHeap(), 0, Buffer);
    }

    va_end(ArgList);
}

//--------------------------------------------------------------------------------------
VOID __cdecl DbgPrintA(PCSZ FormatString, ...)
{
    va_list ArgList;
    va_start(ArgList, FormatString);

    PSTR Buffer = reinterpret_cast<PSTR>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DbgPrintMaxCB));
    if (Buffer != NULL)
    {
        HRESULT hr = StringCbVPrintfA(Buffer, DbgPrintMaxCB, FormatString, ArgList);
        if (SUCCEEDED(hr))
        {
            UNICODE_STRING UnicodeString;
            CreateUnicodeStringA(&UnicodeString, Buffer);
            OutputDebugStringW(UnicodeString.Buffer);       // UNICODE_STRING: は NULL 保障しないので不味いかも
            RtlFreeUnicodeString(&UnicodeString);
        }

        HeapFree(GetProcessHeap(), 0, Buffer);
    }

    va_end(ArgList);
}

//--------------------------------------------------------------------------------------
ULONG NTAPI DbgExceptionInfo(EXCEPTION_POINTERS* e)
{
    DbgPrintW(L"!!!!!!!!!! CATCH EXCEPTION !!!!!!!!!!");
    DbgPrintW(L"TICK: %08X\n", GetTickCount());

    PEXCEPTION_RECORD r = e->ExceptionRecord;
    DbgPrintW(L"CODE: %08X\n", r->ExceptionCode);
    DbgPrintW(L"ADDR: %08X\n", r->ExceptionAddress);

    PCONTEXT c = e->ContextRecord;
    DbgPrintW(L"EAX : %08X\n", c->Eax);
    DbgPrintW(L"EBX : %08X\n", c->Ebx);
    DbgPrintW(L"ECX : %08X\n", c->Ecx);
    DbgPrintW(L"EDX : %08X\n", c->Edx);
    DbgPrintW(L"ESI : %08X\n", c->Esi);
    DbgPrintW(L"EDI : %08X\n", c->Edi);
    DbgPrintW(L"ESP : %08X\n", c->Esp);
    DbgPrintW(L"EBP : %08X\n", c->Ebp);
    DbgPrintW(L"EIP : %08X\n", c->Eip);

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif  // defined(DBG) || defined(_DEBUG)

//--------------------------------------------------------------------------------------
// PrivateHeap を作成して LFH を有効にした HANDLE を返す.
// 作成に失敗したときは GetProcessHeap と同じ.
//--------------------------------------------------------------------------------------
HANDLE NTAPI CreatePrivateHeap(ULONG Options, SIZE_T InitialSize, SIZE_T MaxiumSize)
{
    HANDLE HeapHandle = HeapCreate(Options, InitialSize, MaxiumSize);
    if (HeapHandle == NULL)
        HeapHandle = GetProcessHeap();
    else
    {
        ULONG Value = 2;
        HeapSetInformation(HeapHandle, HeapCompatibilityInformation, &Value, sizeof(ULONG));
    }
    return HeapHandle;
}

//--------------------------------------------------------------------------------------
VOID NTAPI GetLocalTimeAsFileTime(FILETIME* LocalTimeAsFileTime)
{
    FILETIME FileTime;
    GetSystemTimeAsFileTime(&FileTime);
    FileTimeToLocalFileTime(&FileTime, LocalTimeAsFileTime);
}

//--------------------------------------------------------------------------------------
VOID NTAPI SystemTimeToFileTimeAdd(CONST SYSTEMTIME* SystemTime, LONGLONG Add, FILETIME* FileTime)
{
    SystemTimeToFileTime(SystemTime, FileTime);
    ULONGLONG FileTimeValue = (*reinterpret_cast<ULONGLONG*>(FileTime)) + Add;
    RtlCopyMemory(FileTime, &FileTimeValue, sizeof(ULONGLONG));
}

//--------------------------------------------------------------------------------------
#ifdef _WINCRYPT_ENABLED
//--------------------------------------------------------------------------------------
// HexDump を出力する.
// VirtualAlloc でメモリを確保するため、返された PVOID は VirtualFree で開放する.
// Flags は予約で0固定.
//--------------------------------------------------------------------------------------
PVOID NTAPI ConvertBinaryToHexDump(CONST PUCHAR Binary, ULONG BinaryLength, ULONG Flags)
{
    Flags = 0;
    Flags = CRYPT_STRING_HEXASCII|CRYPT_STRING_NOCR;

    ULONG Request = 0;
    PVOID Buffer = NULL;
    BOOL Result = FALSE;

    // サイズ取得
    Result = CryptBinaryToStringW(Binary, BinaryLength, Flags, NULL, &Request);
    if (Result == FALSE)
        return NULL;

    // メモリ確保
    Buffer = VirtualAllocEx(GetCurrentProcess(), NULL, Request<<1, MEM_COMMIT, PAGE_READWRITE);
    if (Buffer == NULL)
        return NULL;

    // Dump
    Result = CryptBinaryToStringW(Binary, BinaryLength, Flags, reinterpret_cast<PWSTR>(Buffer), &Request);
    if (Result == FALSE)
    {
        VirtualFreeEx(GetCurrentProcess(), Buffer, 0, MEM_RELEASE);
        return NULL;
    }

    return Buffer;
}
#endif  // _WINCRYPT_ENABLED

//--------------------------------------------------------------------------------------
#ifdef _GDIPLUS_ENABLED
//--------------------------------------------------------------------------------------
namespace Gdiplus
{
//--------------------------------------------------------------------------------------
// Local variables
//--------------------------------------------------------------------------------------
ULONG_PTR GpToken = 0;

//--------------------------------------------------------------------------------------
VOID NTAPI Startup()
{
    if (GpToken == 0)
    {
        GdiplusStartupInput GpStartupInput;
        GdiplusStartup(&GpToken, &GpStartupInput, NULL);
    }
}

//--------------------------------------------------------------------------------------
VOID NTAPI Shutdown()
{
    if (GpToken)
        GdiplusShutdown(GpToken);
}

//--------------------------------------------------------------------------------------
BOOL NTAPI GetEncoderCLSID(CONST PWSTR MimeType, CLSID* Clsid)
{
    UINT NumEncoders=0, Size=0;

    GetImageEncodersSize(&NumEncoders, &Size);
    if (Size == 0)
        return FALSE;

    BOOL Result = FALSE;

    try
    {
        ImageCodecInfo* Encoders = new ImageCodecInfo[Size];
        GetImageEncoders(NumEncoders, Size, Encoders);

        for (UINT i=0; i<NumEncoders; i++)
        {
            if (wcscmp(Encoders[i].MimeType, MimeType) == 0)
            {
                *Clsid = Encoders[i].Clsid;
                Result = TRUE;
                break;
            }
        }

        delete [] Encoders;
    }
    catch (...)
    {
        throw;
    }

    return Result;
}

};
#endif  // _GDIPLUS_ENABLED


//--------------------------------------------------------------------------------------
#ifdef _WINSDK_BUILD

//--------------------------------------------------------------------------------------
VOID RtlFreeAnsiString(PANSI_STRING AnsiString)
{
    VOID (NTAPI *UmRtlFreeAnsiString)(PANSI_STRING);
    ASSOCIATE_NATIVEAPI(RtlFreeAnsiString);
    UmRtlFreeAnsiString(AnsiString);
}

//--------------------------------------------------------------------------------------
VOID RtlFreeUnicodeString(PUNICODE_STRING UnicodeString)
{
    VOID (NTAPI *UmRtlFreeUnicodeString)(PUNICODE_STRING);
    ASSOCIATE_NATIVEAPI(RtlFreeUnicodeString);
    UmRtlFreeUnicodeString(UnicodeString);
}

//--------------------------------------------------------------------------------------
VOID RtlInitAnsiString(PANSI_STRING DestinationString, PCSZ SourceString)
{
    VOID (NTAPI *UmRtlInitAnsiString)(PANSI_STRING, PCSZ);
    ASSOCIATE_NATIVEAPI(RtlInitAnsiString);
    UmRtlInitAnsiString(DestinationString, SourceString);
}
//--------------------------------------------------------------------------------------
VOID RtlInitUnicodeString(PUNICODE_STRING DestinationString, PCWSTR SourceString)
{
    VOID (NTAPI *UmRtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
    ASSOCIATE_NATIVEAPI(RtlInitUnicodeString);
    UmRtlInitUnicodeString(DestinationString, SourceString);
}

//--------------------------------------------------------------------------------------
NTSTATUS RtlAnsiStringToUnicodeString(PUNICODE_STRING DestinationString, PCANSI_STRING SourceString, BOOLEAN AllocateDestinationString)
{
    NTSTATUS (NTAPI *UmRtlAnsiStringToUnicodeString)(PUNICODE_STRING, PCANSI_STRING, BOOLEAN);
    ASSOCIATE_NATIVEAPI(RtlAnsiStringToUnicodeString);
    return UmRtlAnsiStringToUnicodeString(DestinationString, SourceString, AllocateDestinationString);
}

//--------------------------------------------------------------------------------------
NTSTATUS RtlUnicodeStringToAnsiString(PANSI_STRING DestinationString, PCUNICODE_STRING SourceString, BOOLEAN AllocateDestinationString)
{
    NTSTATUS (NTAPI *UmRtlUnicodeStringToAnsiString)(PANSI_STRING, PCUNICODE_STRING, BOOLEAN);
    ASSOCIATE_NATIVEAPI(RtlUnicodeStringToAnsiString);
    return UmRtlUnicodeStringToAnsiString(DestinationString, SourceString, AllocateDestinationString);
}

#endif  // _WINSDK_BUILD
