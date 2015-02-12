#ifndef DEBUG_H
#define DEBUG_H

#include <wchar.h>

////////////////////////////////////////////////////////////////////////////////


enum DBGPRT { DBGREL, DBGERR, DBGINF, DBGINFL2, DBGHEX, DBGALL, };


#ifdef DBG

void InitDebug(int DebugLevel);
void CloseDebug();
void DbgPrintW(int Level, wchar_t* Format, ...);

#if defined(WIN32) && (NTDDI_VERSION>=NTDDI_WINXP)
void DbgHexPrintW(CONST PBYTE Data, ULONG Length);
#else
DbgHexPrintW	__noop
#endif

#else

#define InitDebug		__noop
#define CloseDebug		__noop
#define DbgPrintW		__noop
#define DbgHexPrintW	__noop

#endif	// #ifdef DBG


#endif	// #ifndef DEBUG_H
