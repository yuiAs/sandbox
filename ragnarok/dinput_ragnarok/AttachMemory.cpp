#include "AttachMemory.hpp"


// warning C4312: 'reinterpret_cast' : conversion from 'DWORD' to 'LPVOID' of greater size
#pragma warning(disable: 4312)

////////////////////////////////////////////////////////////////////////////////


bool CAttachMemory::attachASProtect()
{
	if (m_pExeNTHdr == NULL)
		return false;

	PIMAGE_SECTION_HEADER pDataSect = GetSectionHeader(2);
	DWORD dwDataAddress = m_pExeNTHdr->OptionalHeader.ImageBase + pDataSect->VirtualAddress;

	BYTE p;
	::CopyMemory(&p, reinterpret_cast<LPCVOID>(dwDataAddress), sizeof(BYTE));

	while (1)
	{
		BYTE q;
		::CopyMemory(&q, reinterpret_cast<LPCVOID>(dwDataAddress), sizeof(BYTE));

		if (p != q)
			break;
		else
			p = q;

		::Sleep(0x10);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////


PIMAGE_NT_HEADERS CAttachMemory::GetNTHeaders(HMODULE hModule)
{
	PIMAGE_DOS_HEADER pExeDOSHdr = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
	return reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<LPBYTE>(hModule)+pExeDOSHdr->e_lfanew);
}


PIMAGE_SECTION_HEADER CAttachMemory::GetSectionHeader(int nSection)
{
	PIMAGE_SECTION_HEADER pSectionHdr = reinterpret_cast<PIMAGE_SECTION_HEADER>(m_pExeNTHdr+1);
	return pSectionHdr+nSection;
}

////////////////////////////////////////////////////////////////////////////////


DWORD CAttachMemory::SearchAddress(DWORD dwAddress, DWORD dwSize, LPCTSTR lpTargetStr)
{
	for (DWORD i=dwAddress;i<dwAddress+dwSize; i++)
	{
		if (::lstrcmpi(reinterpret_cast<LPCTSTR>(i), lpTargetStr) == 0)
			break;
	}
	
	return i;
}


DWORD CAttachMemory::SearchAddress(DWORD dwAddress, DWORD dwSize, LPBYTE lpData, size_t Length)
{
	DWORD dwOldProtect;
	::VirtualProtect(reinterpret_cast<LPVOID>(dwAddress), dwSize, PAGE_READWRITE, &dwOldProtect);
	
	for (DWORD i=dwAddress;i<dwAddress+dwSize; i++)
	{
		if (::memcmp(reinterpret_cast<LPVOID>(i), lpData, Length) == 0)
			break;
	}

	// Restore memory protect
	::VirtualProtect(reinterpret_cast<LPVOID>(dwAddress), dwSize, dwOldProtect, &dwOldProtect);

	return i;
}

////////////////////////////////////////////////////////////////////////////////


DWORD CAttachMemory::GetIATBaseAddress()
{
	if (m_pExeNTHdr == NULL)
		return 0;

	return m_pExeNTHdr->OptionalHeader.ImageBase + m_pExeNTHdr->OptionalHeader.BaseOfData;
}

////////////////////////////////////////////////////////////////////////////////

// bool MemoryPatch(
//	DWORD dwAddress,		// starting address
//	LPCVOID lpBuffer,		// Pointer to the buffer
//	SIZE_T Length,			// Number of bytes to be written
//	bool protect			// use VirtualProtect() (default=false)
// );

bool CAttachMemory::MemoryPatch(DWORD dwAddress, LPCVOID lpBuffer, SIZE_T Length, bool protect)
{
	LPVOID lpDestination = reinterpret_cast<LPVOID>(dwAddress);

	if (protect == false)
		CopyMemory(lpDestination, lpBuffer, Length);
	else
	{
		DWORD dwOldProtect;

		if (::VirtualProtect(lpDestination, Length, PAGE_READWRITE, &dwOldProtect) == FALSE)
			return false;

		::CopyMemory(lpDestination, lpBuffer, Length);

		if (::VirtualProtect(lpDestination, Length, dwOldProtect, &dwOldProtect) == FALSE)
			return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////


DWORD CAttachMemory::SearchData(LPCTSTR lpTargetString)
{
	if (m_pExeNTHdr == NULL)
		return 0;

	PIMAGE_SECTION_HEADER pDataSect = GetSectionHeader(2);
	DWORD dwDataAddress = m_pExeNTHdr->OptionalHeader.ImageBase + pDataSect->VirtualAddress;
	DWORD dwTargetAddress = SearchAddress(dwDataAddress, pDataSect->Misc.VirtualSize, lpTargetString);

	return dwTargetAddress;
}


DWORD CAttachMemory::SearchCode(LPBYTE lpData, size_t Length, int shift)
{
	if (m_pExeNTHdr == NULL)
		return 0;

	PIMAGE_SECTION_HEADER pTextSect = GetSectionHeader(0);
	DWORD dwTextAddress = m_pExeNTHdr->OptionalHeader.ImageBase + pTextSect->VirtualAddress;

	DWORD dwAddress = SearchAddress(dwTextAddress, pTextSect->Misc.VirtualSize, lpData, Length) + shift;
	if (dwAddress == 0)
		return 0;

	DWORD dwOldProtect, dwTargetAddress;
	::VirtualProtect(reinterpret_cast<LPVOID>(dwAddress), sizeof(DWORD), PAGE_READWRITE, &dwOldProtect);
	::CopyMemory(&dwTargetAddress, reinterpret_cast<LPCVOID>(dwAddress), sizeof(DWORD));
	::VirtualProtect(reinterpret_cast<LPVOID>(dwAddress), sizeof(DWORD), dwOldProtect, &dwOldProtect);

	return dwTargetAddress;
}

////////////////////////////////////////////////////////////////////////////////


DWORD CAttachMemory::GetCodeAddress(LPBYTE lpData, size_t Length)
{
	if (m_pExeNTHdr == NULL)
		return 0;

	PIMAGE_SECTION_HEADER pTextSect = GetSectionHeader(0);
	DWORD dwTextAddress = m_pExeNTHdr->OptionalHeader.ImageBase + pTextSect->VirtualAddress;

	return SearchAddress(dwTextAddress, pTextSect->Misc.VirtualSize, lpData, Length);
}


DWORD CAttachMemory::GetCallAddress(DWORD shift)
{
	if (m_pExeNTHdr == NULL)
		return 0;

	BYTE code[5];
	code[0] = 0xE8;
	*reinterpret_cast<DWORD*>(code+1) = shift;

	PIMAGE_SECTION_HEADER pTextSect = GetSectionHeader(0);
	DWORD dwTextAddress = m_pExeNTHdr->OptionalHeader.ImageBase + pTextSect->VirtualAddress;

	return SearchAddress(dwTextAddress, pTextSect->Misc.VirtualSize, code, 0x05) + 0x05 + shift;
}
