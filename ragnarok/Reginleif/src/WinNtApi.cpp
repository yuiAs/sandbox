#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "WinNtApi.h"

//------------------------------------------------------------------------------
// Defined
//------------------------------------------------------------------------------
#define ASSOCIATE_NATIVEAPI(api) (FARPROC&)##api = GetProcAddress(GetModuleHandleW(L"NTDLL.DLL"), #api)

//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------
VOID (NTAPI *RtlFreeAnsiString)(PANSI_STRING);
VOID (NTAPI *RtlFreeUnicodeString)(PUNICODE_STRING);
VOID (NTAPI *RtlFreeOemString)(POEM_STRING);
VOID (NTAPI *RtlInitString)(PSTRING, PCSZ);
VOID (NTAPI *RtlInitAnsiString)(PANSI_STRING, PCSZ);
VOID (NTAPI *RtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
NTSTATUS (NTAPI *RtlAnsiStringToUnicodeString)(PUNICODE_STRING, PCANSI_STRING, BOOLEAN);
NTSTATUS (NTAPI *RtlUnicodeStringToAnsiString)(PANSI_STRING, PCUNICODE_STRING, BOOLEAN);
NTSTATUS (NTAPI *RtlUnicodeStringToOemString)(POEM_STRING, PCUNICODE_STRING, BOOLEAN);

//------------------------------------------------------------------------------
//  Function:   InitNativeApi
//  Notes   :   NTDLL.DLLÇÕà√ñŸìIÇ…ïKÇ∏LoadÇ≥ÇÍÇƒÇ¢ÇÈÅB
//------------------------------------------------------------------------------
VOID InitNativeApi()
{
    ASSOCIATE_NATIVEAPI(RtlFreeAnsiString);
    ASSOCIATE_NATIVEAPI(RtlFreeUnicodeString);
    ASSOCIATE_NATIVEAPI(RtlFreeOemString);
    ASSOCIATE_NATIVEAPI(RtlInitString);
    ASSOCIATE_NATIVEAPI(RtlInitAnsiString);
    ASSOCIATE_NATIVEAPI(RtlInitUnicodeString);
    ASSOCIATE_NATIVEAPI(RtlAnsiStringToUnicodeString);
    ASSOCIATE_NATIVEAPI(RtlUnicodeStringToAnsiString);
    ASSOCIATE_NATIVEAPI(RtlUnicodeStringToOemString);
}
