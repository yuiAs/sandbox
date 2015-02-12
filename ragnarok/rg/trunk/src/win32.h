#ifndef WIN32_H
#define WIN32_H

#include <windows.h>
#include "DLinkList.h"
#include "NTTime.hpp"

#ifndef __BUILDMACHINE__
#define RtlInitializeSListHead			InitializeSListHead
#define	RtlInterlockedFlushSList		InterlockedFlushSList
#define RtlInterlockedPopEntrySList		InterlockedPopEntrySList
#define RtlInterlockedPushEntrySList	InterlockedPushEntrySList
#endif

#endif	// #ifndef WIN32_H
