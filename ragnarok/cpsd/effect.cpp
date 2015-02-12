#include <windows.h>
#include "effect.h"

////////////////////////////////////////////////////////////////////////////////


void vprotect(const void* address, MEMORY_BASIC_INFORMATION* pmi)
{
	::VirtualQuery(address, pmi, sizeof(MEMORY_BASIC_INFORMATION));
	::VirtualProtect(pmi->BaseAddress, pmi->RegionSize, PAGE_READWRITE, &pmi->Protect);
}

void vprotect_restore(MEMORY_BASIC_INFORMATION* pmi)
{
	DWORD dummy;
	::VirtualProtect(pmi->BaseAddress, pmi->RegionSize, pmi->Protect, &dummy);
}


void memcpy_f(void* dest, const void* src, size_t count)
{
	MEMORY_BASIC_INFORMATION mi;

	vprotect(dest, &mi);
	RtlCopyMemory(dest, src, count);	// memcpy(dst, src, count);
	vprotect_restore(&mi);
}

////////////////////////////////////////////////////////////////////////////////


void* vm = NULL;
int curvm = 0;


void effect_uncommit()
{
	if (vm)
		::VirtualFree(vm, 0, MEM_RELEASE);
}

////////////////////////////////////////////////////////////////////////////////


bool effect_trampoline()
{
	if (vm == NULL)
	{
		vm = ::VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		memset(vm, 0x90, 4096);
		curvm = 0;
	}

	BYTE code[] = {
		0xA1, 0x00, 0x00, 0x00, 0x00,			// MOV EAX, [00000000]
		0x85, 0xC0,								// TEST EAX, EAX
		0x74, 0x07,								// JE $+7
		0xB8, 0x00, 0x00, 0x00, 0x00,			// MOV EAX, 00000000
		0xFF, 0xE0,								// JMP EAX
		0xB8, 0x00, 0x00, 0x00, 0x00,			// MOV EAX, 00000000
		0xFF, 0xE0,								// JMP EAX
	};
/*
	*reinterpret_cast<DWORD*>(code+1) = 0x00759BA0;
	*reinterpret_cast<DWORD*>(code+10) = 0x005B0AEE;
	*reinterpret_cast<DWORD*>(code+17) = 0x005B031F;
*/
	*reinterpret_cast<DWORD*>(code+1) = 0x007551F8;
	*reinterpret_cast<DWORD*>(code+10) = 0x005ABEAE;
	*reinterpret_cast<DWORD*>(code+17) = 0x005AB6DF;

	memcpy(vm, code, sizeof(code));

	{
		WORD skill = 0x59;
/*
		BYTE flag = *reinterpret_cast<BYTE*>((skill-0x0A)+0x005B0DD0);
		DWORD addr = 0x005B0B54 + flag*4;
*/
		BYTE flag = *reinterpret_cast<BYTE*>((skill-0x0A)+0x005AC190);
		DWORD addr = 0x005ABF14 + flag*4;

		memcpy_f(reinterpret_cast<void*>(addr), &vm, sizeof(DWORD));
	}


	return true;
}
