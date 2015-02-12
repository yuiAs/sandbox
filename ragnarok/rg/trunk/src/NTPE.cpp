#include "NTPE.h"
#include "debug.h"

ULONG g_NTPEDataOffset = 2;

////////////////////////////////////////////////////////////////////////////////

// Memory

void UnlockMemoryProtect(CONST PVOID Address, PMEMORY_BASIC_INFORMATION MemoryInfo)
{
	VirtualQuery(Address, MemoryInfo, sizeof(MEMORY_BASIC_INFORMATION));
	VirtualProtect(MemoryInfo->BaseAddress, MemoryInfo->RegionSize, PAGE_WRITECOPY, &MemoryInfo->Protect);
}

void RevertMemoryProtect(PMEMORY_BASIC_INFORMATION MemoryInfo)
{
	DWORD Current;
	VirtualProtect(MemoryInfo->BaseAddress, MemoryInfo->RegionSize, MemoryInfo->Protect, &Current);
}

void ForceCopyMemory(PVOID Destination, CONST PVOID Source, SIZE_T Length)
{
	MEMORY_BASIC_INFORMATION MemInfo;

	UnlockMemoryProtect(Destination, &MemInfo);
	RtlCopyMemory(Destination, Source, Length);
	RevertMemoryProtect(&MemInfo);
}

////////////////////////////////////////////////////////////////////////////////

// PEImage

PIMAGE_NT_HEADERS GetImageNTHeaders(HANDLE Module)
{
	PIMAGE_DOS_HEADER DOSHdr = reinterpret_cast<PIMAGE_DOS_HEADER>(Module);
	return reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<LONG>(DOSHdr)+DOSHdr->e_lfanew);
}

void GetPESection(ULONG Offset, PULONG Base, PULONG Size)
{
	PIMAGE_NT_HEADERS NTHdr = GetImageNTHeaders(GetModuleHandle(NULL));
	PIMAGE_SECTION_HEADER SectHdr = reinterpret_cast<PIMAGE_SECTION_HEADER>(NTHdr+1) + Offset;

	*Base = NTHdr->OptionalHeader.ImageBase + SectHdr->VirtualAddress;
	*Size = SectHdr->Misc.VirtualSize;
}

inline void GetPEDataSection(PULONG Base, PULONG Size)
{
	GetPESection(2, Base, Size);
}

inline void GetPETextSection(PULONG Base, PULONG Size)
{
	GetPESection(0, Base, Size);
}

// Compare

// Find

bool FindData(ULONG Offset, CONST PUCHAR Data, SIZE_T Length, PULONG Result)
{
	*Result = 0;

	ULONG Base=0, Size=0;
	GetPESection(Offset, &Base, &Size);

	for (ULONG i=Base; i<(Base+Size)-Length; i++)
	{
		__try
		{
			if (RtlEqualMemory(reinterpret_cast<PVOID>(i), Data, Length))
			{
				*Result = i;
				__leave;
			}
		}
		__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
		{
			DbgPrintW(DBGERR, _CRT_WIDE("ERR Raise ACCESS_VIOLATION at FindData, Current=%08X\n"), i);
			DbgPrintW(DBGERR, _CRT_WIDE("ERR Length=%d\n"), Length);
			DbgHexPrintW(Data, Length);
		}

		if (*Result)
			break;
	}

	return (*Result!=0);
}

bool FindBin(CONST PUCHAR Data, SIZE_T Length, PULONG Result)
{
	return FindData(0, Data, Length, Result);
}

bool FindStr(CONST PSTR String, PULONG Result)
{
	return FindData(g_NTPEDataOffset, reinterpret_cast<const PUCHAR>(String), strlen(String), Result);
}

bool FindAddStack(CONST PSTR String, PULONG Result)
{
	if (FindStr(String, Result))
	{
		UCHAR Code[] = {
			0x68, 0x00, 0x00, 0x00, 0x00,	// push	imm32
		};
		*reinterpret_cast<ULONG*>(Code+1) = *Result;

		if (FindBin(Code, sizeof(Code), Result))
			return true;
	}

	*Result = 0;
	return false;
}

// Resolve

bool ResolveEIP(ULONG Address, PULONG Result)
{
	if (CompareBinary<UCHAR>(Address, 0xE8) == false)
		return false;

	ULONG Relative = *reinterpret_cast<ULONG*>(Address+1);	// call	rel32
	*Result = (Address+5) + Relative;

	DbgPrintW(DBGINF, _CRT_WIDE("CHK Resolve EIP=%08X\n"), *Result);
	DbgPrintW(DBGINF, _CRT_WIDE("CHK Base=%08X Relative=%08X Return=%08X\n"), Address, Relative, Address+5);

	return true;
}

////////////////////////////////////////////////////////////////////////////////


void SetDataSection(ULONG Threshold)
{
	PIMAGE_NT_HEADERS NTHdr = GetImageNTHeaders(GetModuleHandle(NULL));

	// offset0‚ÍŠmŽÀ‚É.text‚Ì‚Í‚¸
	for (INT32 i=1; i<NTHdr->FileHeader.NumberOfSections; i++)
	{
		PIMAGE_SECTION_HEADER SectHdr = reinterpret_cast<PIMAGE_SECTION_HEADER>(NTHdr+1) + i;
		if (SectHdr->Misc.VirtualSize >= Threshold)
		{
			g_NTPEDataOffset = i;
			break;
		}
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF PEDataSectionOffset=%d\n"), g_NTPEDataOffset);
}

////////////////////////////////////////////////////////////////////////////////
#ifdef DBG

void DbgNTPE()
{
	PIMAGE_NT_HEADERS NTHdr = GetImageNTHeaders(GetModuleHandle(NULL));

	DbgPrintW(DBGINF, _CRT_WIDE("CHK DbgNTPE\n"));
	DbgPrintW(DBGINF, _CRT_WIDE("CHK FILE::NumberOfSections=%d\n"), NTHdr->FileHeader.NumberOfSections);
	DbgPrintW(DBGINF, _CRT_WIDE("CHK OPTIONAL::ImageBase=%08X\n"), NTHdr->OptionalHeader.ImageBase);
	DbgPrintW(DBGINF, _CRT_WIDE("CHK OPTIONAL::BaseOfCode=%08X\n"), NTHdr->OptionalHeader.BaseOfCode);
	DbgPrintW(DBGINF, _CRT_WIDE("CHK OPTIONAL::BaseOfData=%08X\n"), NTHdr->OptionalHeader.BaseOfData);

	for (INT32 i=0; i<NTHdr->FileHeader.NumberOfSections; i++)
	{
		PIMAGE_SECTION_HEADER SectHdr = reinterpret_cast<PIMAGE_SECTION_HEADER>(NTHdr+1) + i;
		DbgPrintW(DBGINF, _CRT_WIDE("CHK IMAGE_SECTION_HEADER[%d]\n"), i);
		DbgPrintW(DBGINF, _CRT_WIDE("CHK SECTION::Name=%S\n"), SectHdr->Name);
		DbgPrintW(DBGINF, _CRT_WIDE("CHK SECTION::VirtualSize=%08X\n"), SectHdr->Misc.VirtualSize);
		DbgPrintW(DBGINF, _CRT_WIDE("CHK SECTION::VirtualAddress=%08X\n"), SectHdr->VirtualAddress);
	}
}

#endif
