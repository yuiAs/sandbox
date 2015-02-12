#ifndef NTPE_H
#define NTPE_H

#include <windows.h>

////////////////////////////////////////////////////////////////////////////////


template <typename T> bool CompareBinary(ULONG Address, T Value)
{
	return (*reinterpret_cast<T*>(Address)==Value);
}

////////////////////////////////////////////////////////////////////////////////

// NTPE.cpp

void ForceCopyMemory(PVOID Destination, CONST PVOID Source, SIZE_T Length);
void UnlockMemoryProtect(CONST PVOID Address, PMEMORY_BASIC_INFORMATION MemoryInfo);
void RevertMemoryProtect(PMEMORY_BASIC_INFORMATION MemoryInfo);

bool FindBin(CONST PUCHAR Data, SIZE_T Length, PULONG Result);
bool FindStr(CONST PSTR String, PULONG Result);
bool FindAddStack(CONST PSTR String, PULONG Result);

bool ResolveEIP(ULONG Address, PULONG Result);

// for Ragnarok client

#ifdef DBG
void DbgNTPE();
#else
#define DbgNTPE __noop
#endif

void SetDataSection(ULONG Threshold);


#endif	// #ifndef NTPE_H
