//--------------------------------------------------------------------------------------
// File: winumadv.h
//--------------------------------------------------------------------------------------
#ifndef _WINUMADV_H_
#define _WINUMADV_H_


#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
#define _WIN32_WINNT 0x0501
#endif

#ifndef WINUMADV_INLINE
#ifdef __cplusplus
#define WINUMADV_INLINE inline
#else
#define WINUMADV_INLINE __inline
#endif
#endif  // WINUMADV_INLINE

#ifdef __BUILDMACHINE__
#define _WINDDK_BUILD
#else
#define _WINSDK_BUILD
#endif	// __BUILDMACHINE__

#include <winnt.h>
#include <winternl.h>



#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------
NTSTATUS NTAPI CreateAnsiStringW(PANSI_STRING DestinationString, PCWSTR SourceString);
NTSTATUS NTAPI CreateUnicodeStringA(PUNICODE_STRING DestinationString, PCSZ SourceString);

//--------------------------------------------------------------------------------------
#if defined(DBG) || defined(_DEBUG)

VOID __cdecl DbgPrintW(PCWSTR FormatString, ...);
VOID __cdecl DbgPrintA(PCSZ FormatString, ...);

ULONG NTAPI DbgExceptionInfo(EXCEPTION_POINTERS* e);
#define ChkExceptionInfo DbgExceptionInfo

#else

#define DbgPrintW __noop
#define DbgPrintA __noop

WINUMADV_INLINE
ULONG NTAPI RelExceptionInfo(EXCEPTION_POINTERS* e)
{
    return EXCEPTION_EXECUTE_HANDLER;
}
#define ChkExceptionInfo RelExceptionInfo

#endif  // defined(DBG) || defined(_DEBUG)

extern SIZE_T DbgPrintMaxCB;

//--------------------------------------------------------------------------------------
HANDLE NTAPI CreatePrivateHeap(ULONG Options, SIZE_T InitialSize, SIZE_T MaxiumSize);

//--------------------------------------------------------------------------------------
VOID NTAPI GetLocalTimeAsFileTime(FILETIME* LocalTimeAsFileTime);
VOID NTAPI SystemTimeToFileTimeAdd(CONST SYSTEMTIME* SystemTime, LONGLONG Add, FILETIME* FileTime);

WINUMADV_INLINE
ULONGLONG NTAPI SecondToNanoSecond(ULONG Second)
{
    return UInt32x32To64(1000*1000*10, Second);
}

//--------------------------------------------------------------------------------------
#ifdef _WINCRYPT_ENABLED
PVOID NTAPI ConvertBinaryToHexDump(CONST PUCHAR Binary, ULONG BinaryLength, ULONG Flags);
#endif  // _WINCRYPT_ENABLED

#ifdef __cplusplus
}       // extern "C"
#endif

//--------------------------------------------------------------------------------------
#ifdef _GDIPLUS_ENABLED
namespace Gdiplus
{
VOID NTAPI Startup();
VOID NTAPI Shutdown();
BOOL NTAPI GetEncoderCLSID(CONST PWSTR MimeType, CLSID* Clsid);
};
#endif  // _GDIPLUS_ENABLED



#endif  // _WINUMADV_H_
