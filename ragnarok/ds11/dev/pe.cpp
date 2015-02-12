#include "pe.h"

////////////////////////////////////////////////////////////////////////////////


PEOp::PEOp()
	: m_nt(NULL)
{
}


PEOp::~PEOp()
{
	destroy();
}

////////////////////////////////////////////////////////////////////////////////


void PEOp::init(HMODULE module)
{
	IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(module);
	m_nt = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<LONG>(dos)+dos->e_lfanew);

	x86_init(opt_none, NULL, NULL);
}


void PEOp::destroy()
{
	x86_cleanup();
}

////////////////////////////////////////////////////////////////////////////////

// FORCE MEMCPY

void PEOp::memcpy_f(void* dest, const void* src, size_t count)
{
	MEMORY_BASIC_INFORMATION mi;

	vprotect(dest, &mi);
	RtlCopyMemory(dest, src, count);	// memcpy(dst, src, count);
	vprotect_restore(&mi);
}


void PEOp::vprotect(const void* address, MEMORY_BASIC_INFORMATION* pmi)
{
	::VirtualQuery(address, pmi, sizeof(MEMORY_BASIC_INFORMATION));
	::VirtualProtect(pmi->BaseAddress, pmi->RegionSize, PAGE_READWRITE, &pmi->Protect);
}


void PEOp::vprotect_restore(MEMORY_BASIC_INFORMATION* pmi)
{
	DWORD dummy;
	::VirtualProtect(pmi->BaseAddress, pmi->RegionSize, pmi->Protect, &dummy);
}

////////////////////////////////////////////////////////////////////////////////


DWORD PEOp::findstr(const char* text)
{
	DWORD address = 0;
	find_data(text, &address);

	return address;
}


DWORD PEOp::findbin(const BYTE* buf, size_t buflen)
{
	DWORD address = 0;
	find_text(buf, buflen, &address);

	return address;
}

////////////////////////////////////////////////////////////////////////////////


bool PEOp::find_data(const char* text, DWORD* address)
{
	IMAGE_SECTION_HEADER* sect = reinterpret_cast<IMAGE_SECTION_HEADER*>(m_nt+1) + 2;
	DWORD st = m_nt->OptionalHeader.ImageBase + sect->VirtualAddress;
	DWORD ed = st + sect->Misc.VirtualSize;

	for (DWORD i=st; i<ed;)
	{
		__try
		{
			const char* current = reinterpret_cast<char*>(i);

			if (strcmp(text, current))
				i += strlen(current) + 1;
			else
			{
				*address = i;
				return true;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			i++;
			continue;
		}
	}

	*address = 0;
	return false;
}


bool PEOp::find_text(const BYTE* buf, size_t buflen, DWORD* address)
{
	IMAGE_SECTION_HEADER* sect = reinterpret_cast<IMAGE_SECTION_HEADER*>(m_nt+1);
	DWORD st = m_nt->OptionalHeader.ImageBase + sect->VirtualAddress;
	DWORD ed = st + sect->Misc.VirtualSize;

	for (DWORD i=st; i<ed; i++)
	{
		__try
		{
			if (memcmp(buf, reinterpret_cast<const BYTE*>(i), buflen) == 0)
			{
				*address = i;
				return true;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			continue;
		}
	}

	*address = 0;
	return false;
}
