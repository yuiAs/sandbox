#ifndef _WINNTAPI_
#define _WINNTAPI_

#ifdef __BUILDMACHINE__
#define _WINDDK_BUILD_
#else
#define _WINSDK_BUILD_
#endif

#ifdef _WINDDK_BUILD_
#else
#include <windef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// These data structures and type definitions are needed for compilation and
// use of the internal Windows APIs defined in this header.
//
typedef __success(return >= 0) LONG NTSTATUS;

typedef CONST char *PCSZ;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;
typedef PSTRING PCANSI_STRING;

typedef STRING OEM_STRING;
typedef PSTRING POEM_STRING;
typedef CONST STRING* PCOEM_STRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

#ifdef _WINDDK_BUILD_

VOID
RtlFreeAnsiString (
    PANSI_STRING AnsiString
    );

VOID
RtlFreeUnicodeString (
    PUNICODE_STRING UnicodeString
    );

VOID
RtlFreeOemString(
    POEM_STRING OemString
    );

VOID
RtlInitString (
    PSTRING DestinationString,
    PCSZ SourceString
    );

VOID
RtlInitAnsiString (
    PANSI_STRING DestinationString,
    PCSZ SourceString
    );

VOID
RtlInitUnicodeString (
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

NTSTATUS
RtlAnsiStringToUnicodeString (
    PUNICODE_STRING DestinationString,
    PCANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSTATUS
RtlUnicodeStringToAnsiString (
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSTATUS
RtlUnicodeStringToOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

#else

// _WINSDK_BUILD_
VOID InitNativeApi();

// WinNtApi.cpp
extern VOID (NTAPI *RtlFreeAnsiString)(PANSI_STRING);
extern VOID (NTAPI *RtlFreeUnicodeString)(PUNICODE_STRING);
extern VOID (NTAPI *RtlFreeOemString)(POEM_STRING);
extern VOID (NTAPI *RtlInitString)(PSTRING, PCSZ);
extern VOID (NTAPI *RtlInitAnsiString)(PANSI_STRING, PCSZ);
extern VOID (NTAPI *RtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
extern NTSTATUS (NTAPI *RtlAnsiStringToUnicodeString)(PUNICODE_STRING, PCANSI_STRING, BOOLEAN);
extern NTSTATUS (NTAPI *RtlUnicodeStringToAnsiString)(PANSI_STRING, PCUNICODE_STRING, BOOLEAN);
extern NTSTATUS (NTAPI *RtlUnicodeStringToOemString)(POEM_STRING, PCUNICODE_STRING, BOOLEAN);

#endif  // _WINDDK_BUILD_

#ifdef __cplusplus
}
#endif

#endif  // _WINNTAPI_
