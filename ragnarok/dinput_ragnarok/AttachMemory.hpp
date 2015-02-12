#ifndef ATTACHMEMORY_HPP
#define ATTACHMEMORY_HPP


#include <windows.h>
#include <tchar.h>


// PEÉÅÉÇÉäì‡ÇòMÇ¡ÇΩÇËåüçıÇµÇΩÇË
// Ragnarokà»äOÇ≈Ç‡égÇ¶ÇªÇ§ÇæÇØÇ«égÇÌÇ»Ç≥ÇªÇ§


class CAttachMemory
{
	PIMAGE_NT_HEADERS m_pExeNTHdr;

public:

	CAttachMemory() { m_pExeNTHdr = GetNTHeaders(GetModuleHandle(NULL)); }
	~CAttachMemory() {}

private:

	PIMAGE_NT_HEADERS GetNTHeaders(HMODULE hModule);
	PIMAGE_SECTION_HEADER GetSectionHeader(int nSection);

	DWORD SearchAddress(DWORD dwAddress, DWORD dwSize, LPCTSTR lpTargetStr);
	DWORD SearchAddress(DWORD dwAddress, DWORD dwSize, LPBYTE lpData, size_t Length);

	bool attachASProtect();

public:

	DWORD GetIATBaseAddress();

	DWORD SearchData(LPCTSTR lpTargetString);
	DWORD SearchCode(LPBYTE lpData, size_t Length, int shift);

	DWORD GetCodeAddress(LPBYTE lpData, size_t Length);
	DWORD GetCallAddress(DWORD shift);
	
	bool MemoryPatch(DWORD dwAddress, LPCVOID lpBuffer, SIZE_T Length, bool protect=false);
};


#endif	// #ifndef ATTACHMEMORY_HPP
