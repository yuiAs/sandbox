#pragma once

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>


#pragma warning(disable: 4311)	// pointer truncation
#pragma warning(disable: 4312)	// conversion of greater size

////////////////////////////////////////////////////////////////////////////////

// ntdll.DbgPrint

#ifdef _DEBUG

static void DbgPrint(const TCHAR* format, ...)
{
	va_list ap;
	va_start(ap, format);

	int length = _vsctprintf(format, ap) + 1;
	TCHAR* buffer = new TCHAR[length];
	//memset(buffer, 0, length);
	_vsntprintf(buffer, length, format, ap);
	::OutputDebugString(buffer);

	va_end(ap);
	delete [] buffer;
}

#else

static void DbgPrint(TCHAR* format, ...) {}

#endif
