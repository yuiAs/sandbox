#include "pe_analysis.hpp"


////////////////////////////////////////////////////////////////////////////////

// パターンマッチcore

// .text

DWORD CPEAnalysis::_searchText(const LPBYTE data, size_t length)
{
	PIMAGE_SECTION_HEADER section = _GetSectionHeader(0);
	DWORD address = m_nt->OptionalHeader.ImageBase + section->VirtualAddress;
	DWORD size = address + section->Misc.VirtualSize;

	DWORD protect;
	::VirtualProtect(reinterpret_cast<LPVOID>(address), section->Misc.VirtualSize, PAGE_READWRITE, &protect);

	for (DWORD i=address; i<size; i++)
	{
		if (memcmp(reinterpret_cast<const void*>(i), data, length) == 0)
		{
			// restore
			::VirtualProtect(reinterpret_cast<LPVOID>(address), section->Misc.VirtualSize, protect, &protect);
			return i;
		}
	}

	// restore
	::VirtualProtect(reinterpret_cast<LPVOID>(address), section->Misc.VirtualSize, protect, &protect);
	return -1;
}

// .data

DWORD CPEAnalysis::_searchData(const TCHAR* string)
{
	PIMAGE_SECTION_HEADER section = _GetSectionHeader(2);
	DWORD address = m_nt->OptionalHeader.ImageBase + section->VirtualAddress;
	DWORD size = address + section->Misc.VirtualSize;

	for (DWORD i=address; i<size; i++)
	{
		if (::lstrcmpi(reinterpret_cast<const TCHAR*>(i), string) == 0)
			return i;
	}

	return -1;
}

////////////////////////////////////////////////////////////////////////////////

// .textからパターンマッチ

DWORD CPEAnalysis::searchCode(const LPBYTE data, size_t length, int shift)
{
	DWORD text = _searchText(data, length);
	if (text == -1)
		return 0;

	text += shift;

	DWORD protect, address;

	::VirtualProtect(reinterpret_cast<LPVOID>(text), sizeof(DWORD), PAGE_READWRITE, &protect);
	::CopyMemory(&address, reinterpret_cast<const void*>(text), sizeof(DWORD));
	::VirtualProtect(reinterpret_cast<LPVOID>(text), sizeof(DWORD), protect, &protect);

	return address;
}

// .dataからパターンマッチ

DWORD CPEAnalysis::searchData(const TCHAR* string)
{
	DWORD data = _searchData(string);
	if (data == -1)
		return 0;

	return data;
}

// address

DWORD CPEAnalysis::searchAddress(const LPBYTE data, size_t length)
{
	DWORD text = _searchText(data, length);
	if (text == -1)
		return 0;

	return text;
}

////////////////////////////////////////////////////////////////////////////////

// patch

bool CPEAnalysis::memoryPatch(DWORD address, void* data, size_t length, bool protect)
{
	return true;
}
