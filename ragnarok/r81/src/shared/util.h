//! @file   util.h
#pragma once


void GetLocalTimeAsFileTime(FILETIME* LocalTimeAsFileTime);
void SystemTimeToFileTimeAdd(CONST SYSTEMTIME* SystemTime, LONGLONG Add, FILETIME* FileTime);
