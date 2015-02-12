#ifndef PE_ANALYSIS_HPP
#define PE_ANALYSIS_HPP


#include <windows.h>
#include <tchar.h>


class CPEAnalysis
{
	PIMAGE_NT_HEADERS m_nt;

public:

	CPEAnalysis() { m_nt = _GetPEHeader(reinterpret_cast<DWORD>(::GetModuleHandle(NULL))); }
	~CPEAnalysis();

private:

	PIMAGE_NT_HEADERS _GetPEHeader(DWORD module)
		{ return reinterpret_cast<PIMAGE_NT_HEADERS>(module+reinterpret_cast<PIMAGE_DOS_HEADER>(module)->e_lfanew); }
	PIMAGE_SECTION_HEADER _GetSectionHeader(int section)
		{ return reinterpret_cast<PIMAGE_SECTION_HEADER>(m_nt+1)+section; }

	// パターンマッチcore

	DWORD _searchText(const LPBYTE data, size_t length);
	DWORD _searchData(const TCHAR* string);

public:

	DWORD IATBase() const { return m_nt->OptionalHeader.ImageBase + m_nt->OptionalHeader.BaseOfCode; }
	// WIN64だとBaseOfCodeは消滅

	// 検索系

	DWORD searchCode(const LPBYTE data, size_t length, int shift);	// .text
	DWORD searchData(const TCHAR* string);							// .data
	DWORD searchAddress(const LPBYTE data, size_t length);			// 

	bool memoryPatch(DWORD address, void* data, size_t length, bool protect=true);
};


#endif	// #ifndef PE_ANALYSIS_HPP
