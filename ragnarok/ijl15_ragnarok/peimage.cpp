#include "peimage.hpp"


////////////////////////////////////////////////////////////////////////////////


void PEImage::initialize()
{
	IMAGE_DOS_HEADER* _dos = reinterpret_cast<IMAGE_DOS_HEADER*>(m_handle);
	m_nt = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<LONG>(_dos)+_dos->e_lfanew);

	clearFF25();
}


void PEImage::getSectionRange(int section, DWORD &start, DWORD &end)
{
	IMAGE_SECTION_HEADER* _sect = reinterpret_cast<IMAGE_SECTION_HEADER*>(m_nt+1) + section;
	start = m_nt->OptionalHeader.ImageBase + _sect->VirtualAddress;
	end = start + _sect->Misc.VirtualSize;
}

////////////////////////////////////////////////////////////////////////////////


bool PEImage::waitASProtect()
{
	DWORD addr_entry = m_nt->OptionalHeader.ImageBase + m_nt->OptionalHeader.AddressOfEntryPoint;
	DbgPrint(_T("address_entry=%08X"), addr_entry);

	while (*reinterpret_cast<const BYTE*>(addr_entry) != 0x55)	// push ebp
		::Sleep(0x10);

	return true;
}

////////////////////////////////////////////////////////////////////////////////


void PEImage::copyMemory(void* dest, const void* source, size_t length)
{
	MEMORY_BASIC_INFORMATION memInfo;

	::VirtualQuery(reinterpret_cast<const void*>(dest), &memInfo, sizeof(MEMORY_BASIC_INFORMATION));
	::VirtualProtect(memInfo.BaseAddress, memInfo.RegionSize, PAGE_READWRITE, &memInfo.Protect);

	::RtlCopyMemory(dest, source, length);

	DWORD dummy;
	::VirtualProtect(memInfo.BaseAddress, memInfo.RegionSize, memInfo.Protect, &dummy);
}

////////////////////////////////////////////////////////////////////////////////


void PEImage::displayFF25()
{
	for (std::map<DWORD, DWORD>::iterator i=m_jmptbl.begin(); i!=m_jmptbl.end(); i++)
		DbgPrint(_T("FF25 [%08X]=%08X"), i->second, i->first);
}


void PEImage::analysisFF25()
{
	IMAGE_SECTION_HEADER* _sect = reinterpret_cast<IMAGE_SECTION_HEADER*>(m_nt+1);
	DWORD addr_start = m_nt->OptionalHeader.ImageBase + _sect->VirtualAddress;
	DWORD addr_end = addr_start + _sect->Misc.VirtualSize;


	MEMORY_BASIC_INFORMATION memInfo;

	::VirtualQuery(reinterpret_cast<const void *>(addr_start), &memInfo, sizeof(MEMORY_BASIC_INFORMATION));
	::VirtualProtect(memInfo.BaseAddress, memInfo.RegionSize, PAGE_READWRITE, &memInfo.Protect);


	BYTE cj[2];
	cj[0] = 0xFF;
	cj[1] = 0x25;	// jmp [________]

	for (DWORD i=addr_start; i<addr_end; i++)
	{
		if (memcmp(reinterpret_cast<const void *>(i), cj, 2) == 0)
		{
			DWORD d = *reinterpret_cast<DWORD*>(i+2);

			if (::IsBadReadPtr(reinterpret_cast<const void*>(d), sizeof(DWORD)) == 0)
			{
				DWORD e = *reinterpret_cast<DWORD*>(d);
				m_jmptbl.insert(std::pair<DWORD, DWORD>(e, d));

			}
		}
	}


	DWORD dummy;
	::VirtualProtect(memInfo.BaseAddress, memInfo.RegionSize, memInfo.Protect, &dummy);
}


void* PEImage::rewriteFF25(const TCHAR* module, const char* export, void* function)
{
	void* original = ::GetProcAddress(::GetModuleHandle(module), export);
	if (original == NULL)
		return 0;

	std::map<DWORD, DWORD>::iterator i = m_jmptbl.find(reinterpret_cast<DWORD>(original));
	if (i == m_jmptbl.end())
		return 0;

	DbgPrint(_T("%s.%s=%08X [%08X]=%08X to %08X"), module, export, original, i->second, i->first, function);
	copyMemory(reinterpret_cast<void*>(i->second), &function, sizeof(DWORD));
	//*reinterpret_cast<DWORD*>(i->second) = reinterpret_cast<DWORD>(function);

	return original;
}


DWORD PEImage::searchFF25(const TCHAR* module, const char* export)
{
	void* original = ::GetProcAddress(::GetModuleHandle(module), export);
	if (original == NULL)
		return 0;

	std::map<DWORD, DWORD>::iterator i = m_jmptbl.find(reinterpret_cast<DWORD>(original));
	if (i != m_jmptbl.end())
		return i->second;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

// .data

DWORD PEImage::matchFirstByData(const char* string)
{
	IMAGE_SECTION_HEADER* _sect = reinterpret_cast<IMAGE_SECTION_HEADER*>(m_nt+1) + 2;
	DWORD addr_start = m_nt->OptionalHeader.ImageBase + _sect->VirtualAddress;
	DWORD addr_end = addr_start + _sect->Misc.VirtualSize;

	for (DWORD i=addr_start; i<addr_end; i++)
	{
		if (strcmp(string, reinterpret_cast<const char*>(i)) == 0)
			return i;
	}

	return -1;
}

// .text

DWORD PEImage::matchFirstByText(const BYTE* data, size_t length)
{
	IMAGE_SECTION_HEADER* _sect = reinterpret_cast<IMAGE_SECTION_HEADER*>(m_nt+1);
	DWORD addr_start = m_nt->OptionalHeader.ImageBase + _sect->VirtualAddress;
	DWORD addr_end = addr_start + _sect->Misc.VirtualSize;

	for (DWORD i=addr_start; i<addr_end; i++)
	{
		if (memcmp(data, reinterpret_cast<const void*>(i), length) == 0)
			return i;
	}

	return -1;
}


// matchFirstBy???Œninterface

DWORD PEImage::searchCode(const BYTE* data, size_t length)
{
	DWORD address = matchFirstByText(data, length);
	if (address == -1)
		return 0;

	return address;
}


DWORD PEImage::searchCodeRef(const BYTE* data, size_t length, size_t shift)
{
	DWORD address = matchFirstByText(data, length);
	if (address == -1)
		return 0;
	else
		address += shift;


	if (::IsBadReadPtr(reinterpret_cast<const void*>(address), sizeof(DWORD)) == 0)
		return *reinterpret_cast<DWORD*>(address);

	return 0;
}


DWORD PEImage::searchString(const char* string)
{
	DWORD address = matchFirstByData(string);
	if (address == -1)
		return 0;

	return address;
}

