#ifndef _WINADVAPI_
#define _WINADVAPI_

#include "WinNtApi.h"

#ifndef _L
#define _L(String) L##String
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Debug
#if (defined(DBG)||defined(_CHKBUILD))
VOID DbgPrintW(CONST PWSTR Format, ...);
VOID DbgPrintA(CONST PSTR Format, ...);
#else
#define DbgPrintW __noop
#define DbgPrintA __noop
#endif
ULONG WINAPI OutputExceptionInfo(EXCEPTION_POINTERS* e);

// Heap
HANDLE WINAPI CreatePrivateHeap(ULONG Options, SIZE_T InitialSize, SIZE_T MaxiumSize);

// Time
VOID WINAPI GetLocalTimeAsFileTime(FILETIME* LocalTimeAsFileTime);
ULONGLONG WINAPI SecondToNanoSecond(ULONG Second);
VOID WINAPI SystemTimeToFileTimeAdd(CONST SYSTEMTIME* SystemTime, LONGLONG Add, FILETIME* FileTime);

// Crypt
#if (_WIN32_WINNT >= 0x0501)

#include <Wincrypt.h>
#pragma comment(lib, "CRYPT32.LIB")

#define _ENABLE_WINCRYPTAPI_

PVOID WINAPI CreateHexDumpW(CONST PUCHAR Data, ULONG Length, ULONG Flags);
VOID WINAPI ReleaseHexDump(PVOID Buffer);

#else

#define CreateHexDumpW __noop
#define ReleaseHexDump __noop

#endif  // (_WIN32_WINNT >= 0x0501)

// RtlString
NTSTATUS NTAPI CreateAnsiStringW(PANSI_STRING DestinationString, PWSTR SourceString);
NTSTATUS NTAPI CreateUnicodeStringA(PUNICODE_STRING DestinationString, PSTR SourceString);

/*++

LINK list:

    Definitions for a double link list.

--*/

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) \
    ((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))
#endif


#ifndef InitializeListHead
//
//  VOID
//  InitializeListHead(
//      PLIST_ENTRY ListHead
//      );
//

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

//
//  PLIST_ENTRY
//  RemoveTailList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

//
//  VOID
//  InsertHeadList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }



BOOL IsNodeOnList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry);


#endif //InitializeListHead

#ifdef __cplusplus
}
#endif

#endif  // _WINADVAPI_
