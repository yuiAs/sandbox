#include "pe_jump_table.hpp"

////////////////////////////////////////////////////////////////////////////////


void PEJmpTbl::build(DWORD start, DWORD end)
{
	MEMORY_BASIC_INFORMATION memInfo;
	_protect(reinterpret_cast<const void *>(start), &memInfo);


/*
	BYTE c[2];
	c[0] = 0xFF;
	c[1] = 0x25;	// jmp [________]
*/
	BYTE c[] = {
		0xFF, 0x25,		// jmp
	};
	
	for (DWORD i=start; i<end; i++)
	{
		if (memcmp(reinterpret_cast<const void *>(i), c, 2) == 0)
		{
			DWORD d = *reinterpret_cast<DWORD*>(i+2);

			if (::IsBadReadPtr(reinterpret_cast<const void*>(d), sizeof(DWORD)) == 0)
			{
				DWORD e = *reinterpret_cast<DWORD*>(d);
				m_table.insert(std::pair<DWORD, DWORD>(e, d));
			}
		}
	}


	_recover(&memInfo);
}

////////////////////////////////////////////////////////////////////////////////


DWORD PEJmpTbl::search(const char* module, const char* exname)
{
	void* original = ::GetProcAddress(::GetModuleHandle(module), exname);
	if (original == NULL)
		return 0;

	std::map<DWORD, DWORD>::iterator i = m_table.find(reinterpret_cast<DWORD>(original));
	if (i != m_table.end())
		return i->second;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


void* PEJmpTbl::rewrite(const char* module, const char* exname, void* function)
{
	void* original = ::GetProcAddress(::GetModuleHandle(module), exname);
	if (original == NULL)
		return 0;

	std::map<DWORD, DWORD>::iterator i = m_table.find(reinterpret_cast<DWORD>(original));
	if (i == m_table.end())
		return 0;


	MEMORY_BASIC_INFORMATION memInfo;

	_protect(reinterpret_cast<const void*>(i->second), &memInfo);
	::RtlCopyMemory(reinterpret_cast<void*>(i->second), &function, sizeof(DWORD));
	_recover(&memInfo);

	
	return original;
}

////////////////////////////////////////////////////////////////////////////////


void PEJmpTbl::_protect(const void* address, MEMORY_BASIC_INFORMATION* memInfo)
{
	::VirtualQuery(address, memInfo, sizeof(MEMORY_BASIC_INFORMATION));
	::VirtualProtect(memInfo->BaseAddress, memInfo->RegionSize, PAGE_READWRITE, &memInfo->Protect);
}


void PEJmpTbl::_recover(MEMORY_BASIC_INFORMATION* memInfo)
{
	DWORD dummy;
	::VirtualProtect(memInfo->BaseAddress, memInfo->RegionSize, memInfo->Protect, &dummy);
}

