#ifndef NTTIME_HPP
#define NTTIME_HPP

#include <windows.h>

////////////////////////////////////////////////////////////////////////////////


static void SystemTimeToFileTimeAdd(CONST SYSTEMTIME* SystemTime, ULONGLONG Add, FILETIME* FileTime)
{
	SystemTimeToFileTime(SystemTime, FileTime);

	ULONGLONG Value = *reinterpret_cast<ULONGLONG*>(FileTime) + Add;
	*FileTime = *reinterpret_cast<FILETIME*>(&Value);
}


static void GetLocalTimeAsFileTime(FILETIME* LocalTimeAsFileTime)
{
	FILETIME FileTime;
	GetSystemTimeAsFileTime(&FileTime);
	FileTimeToLocalFileTime(&FileTime, LocalTimeAsFileTime);
}


inline ULONGLONG ConvertSecondToNanoSecond(ULONG Second)
{
	return UInt32x32To64(1000*1000*10, Second);	// Macro
}


#endif	// #ifndef NTTIME_HPP
