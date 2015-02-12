//! @file   debug.h
#pragma once

#if !defined(_USRDBGBUF)
#define _USRDBGBUF 512
#endif


ULONG DbgExceptionInfo(EXCEPTION_POINTERS* e);

void UsrDbgPringA(const char* FormatString, ...);
void UsrDbgPrintW(const wchar_t* FormatString, ...);
void UsrDbgDump(const PUCHAR Binary, SIZE_T BinaryLength, ULONG Flags);

#if !defined(DbgPrintW)
#define DbgPrintW UsrDbgPrintW
#endif
#if !defined(DbgPrintA)
#define DbgPrintA UsrDbgPrintA
#endif
#if !defined(DbgDump)
#define DbgDump UsrDbgDump
#endif

PVOID ConvertBinaryToHexDump(const PUCHAR Binary, SIZE_T BinaryLength, ULONG Flags);

#if defined(_USRDBG_CLEANUP)
#define DbgPrintW __noop
#define DbgPrintA __noop
#define DbgDump __noop
#define ConvertBinaryToHexDump __noop
#endif
