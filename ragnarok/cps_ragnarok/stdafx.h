#pragma once

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>


#pragma warning(disable: 4311)	// pointer truncation
#pragma warning(disable: 4312)	// conversion of greater size

////////////////////////////////////////////////////////////////////////////////

// ntdll.DbgPrint

#ifdef _DEBUG
#include <atlstr.h>

static void DbgPrint(TCHAR* format, ...)
{
	CAtlString dstring;
	va_list ap;

	va_start(ap, format);
	dstring.FormatV(format, ap);
	va_end(ap);

	::OutputDebugString(dstring);
}

#else

static void DbgPrint(TCHAR* format, ...) {}

#endif
