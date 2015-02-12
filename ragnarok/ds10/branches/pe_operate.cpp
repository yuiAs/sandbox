#include "pe_operate.hpp"

////////////////////////////////////////////////////////////////////////////////


PEOp::~PEOp()
{
	if (m_jmpTbl)
		delete m_jmpTbl;
}


void PEOp::initialize(HMODULE __module)
{
	IMAGE_DOS_HEADER* _dos = reinterpret_cast<IMAGE_DOS_HEADER*>(__module);
	m_NT = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<LONG>(_dos)+_dos->e_lfanew);

	m_jmpTbl = new PEJmpTbl;
}

////////////////////////////////////////////////////////////////////////////////

// wait expanding for ASProtect

bool PEOp::wait_ASProtect(DWORD loop_wait, DWORD expand_wait)
{
	DWORD entry = m_NT->OptionalHeader.ImageBase + m_NT->OptionalHeader.AddressOfEntryPoint;

	while (*reinterpret_cast<const BYTE*>(entry) != 0x55)	// push ebp
		::Sleep(loop_wait);

	::Sleep(expand_wait);
	return true;
}

////////////////////////////////////////////////////////////////////////////////

// wrap RtlCopyMemory

void PEOp::copy(void* dest, const void* source, size_t length)
{
	MEMORY_BASIC_INFORMATION memInfo;

	_protect(reinterpret_cast<const void*>(dest), &memInfo);
	::RtlCopyMemory(dest, source, length);
	_recover(&memInfo);
}


void PEOp::_protect(const void* address, MEMORY_BASIC_INFORMATION* memInfo)
{
	::VirtualQuery(address, memInfo, sizeof(MEMORY_BASIC_INFORMATION));
	::VirtualProtect(memInfo->BaseAddress, memInfo->RegionSize, PAGE_READWRITE, &memInfo->Protect);
}


void PEOp::_recover(MEMORY_BASIC_INFORMATION* memInfo)
{
	DWORD dummy;
	::VirtualProtect(memInfo->BaseAddress, memInfo->RegionSize, memInfo->Protect, &dummy);
}

////////////////////////////////////////////////////////////////////////////////


DWORD PEOp::code(const BYTE* buf, size_t buflen)
{
	DWORD address = _match_first_at_text(buf, buflen);
	if (address != -1)
		return address;

	return 0;
}


DWORD PEOp::code_ref(const BYTE* buf, size_t buflen, int shift)
{
	DWORD address = _match_first_at_text(buf, buflen);
	if (address != -1)
	{
		address += shift;

		if (::IsBadReadPtr(reinterpret_cast<const void*>(address), sizeof(DWORD)) == 0)
			return *reinterpret_cast<DWORD*>(address);
	}

	return 0;
}


DWORD PEOp::string(const char* text)
{
	DWORD address = _match_first_at_data(text);
	if (address != -1)
		return address;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


DWORD PEOp::_match_first_at_data(const char* text)
{
	IMAGE_SECTION_HEADER* sect = reinterpret_cast<IMAGE_SECTION_HEADER*>(m_NT+1) + 2;
	DWORD start = m_NT->OptionalHeader.ImageBase + sect->VirtualAddress;
	DWORD end = start + sect->Misc.VirtualSize;

	for (DWORD i=start; i<end;)
	{
		if (strcmp(text, reinterpret_cast<const char*>(i)) == 0)
			return i;
		else
			i += strlen(reinterpret_cast<const char*>(i))+1;
	}

	return -1;
}


DWORD PEOp::_match_first_at_text(const BYTE* buf, size_t buflen)
{
	IMAGE_SECTION_HEADER* sect = reinterpret_cast<IMAGE_SECTION_HEADER*>(m_NT+1);
	DWORD start = m_NT->OptionalHeader.ImageBase + sect->VirtualAddress;
	DWORD end = start + sect->Misc.VirtualSize;

	for (DWORD i=start; i<end; i++)
	{
		if (memcmp(buf, reinterpret_cast<const void*>(i), buflen) == 0)
			return i;
	}

	return -1;
}

////////////////////////////////////////////////////////////////////////////////

// PEJmpTbl interface

void PEOp::jmp_build()
{
	if (m_jmpTbl->isBuild() == false)
	{
		IMAGE_SECTION_HEADER* sect = reinterpret_cast<IMAGE_SECTION_HEADER*>(m_NT+1);
		DWORD start = m_NT->OptionalHeader.ImageBase + sect->VirtualAddress;
		DWORD end = start + sect->Misc.VirtualSize;

		m_jmpTbl->build(start, end);
	}
}

void PEOp::jmp_clear()
{
	m_jmpTbl->clear();
}

DWORD PEOp::jmp_search(const char* module, const char* exname)
{
	return m_jmpTbl->search(module, exname);
}

void* PEOp::jmp_rewrite(const char* module, const char* exname, void* function)
{
	return m_jmpTbl->rewrite(module, exname, function);
}
