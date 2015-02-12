//! @file   util.cpp

#include <windows.h>
#include "util.h"


//! -
void GetLocalTimeAsFileTime(FILETIME* LocalTimeAsFileTime)
{
    FILETIME filetime;
    GetSystemTimeAsFileTime(&filetime);
    FileTimeToLocalFileTime(&filetime, LocalTimeAsFileTime);
}

//! -
void SystemTimeToFileTimeAdd(CONST SYSTEMTIME* SystemTime, LONGLONG AddValue, FILETIME* FileTime)
{
    SystemTimeToFileTime(SystemTime, FileTime);
    ULONGLONG FileTimeValue = (*reinterpret_cast<ULONGLONG*>(FileTime)) + AddValue;
    RtlCopyMemory(FileTime, &FileTimeValue, sizeof(ULONGLONG));
}
