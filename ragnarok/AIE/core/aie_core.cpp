#include <windows.h>
#include <tchar.h>
#include <atlstr.h>
#include "apiHook.hpp"


////////////////////////////////////////////////////////////////////////////////


void debug_out(TCHAR* format, ...)
{
	CAtlString dstring;
	va_list ap;

	va_start(ap, format);
	dstring.FormatV(format, ap);
	va_end(ap);

	::OutputDebugString(dstring);
}

////////////////////////////////////////////////////////////////////////////////


DWORD u32_gwtpid=0, k32_op=0, k32_rpm=0;
//void *u32_gwtpid=0, *k32_op=0, *k32_rpm=0;

__declspec(naked) DWORD __stdcall _gwtpi(HWND wnd, DWORD* pid)
{
	__asm
	{
		jmp u32_gwtpid
	}
}


__declspec(naked) HANDLE __stdcall _op(DWORD access, BOOL inheritable, DWORD pid)
{
	__asm
	{
		jmp k32_op
	}
}


__declspec(naked) BOOL __stdcall _rpm(HANDLE process, const void* address, void* buffer, SIZE_T size, SIZE_T* readsize)
{
	__asm
	{
		jmp k32_rpm
	}
}

////////////////////////////////////////////////////////////////////////////////


//DWORD valloc = 0;
void* valloc = 0;

/*
__declspec(naked) LPVOID __stdcall _valloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
	__asm
	{
		mov edi,edi
		push ebp
		mov ebp,esp
		jmp valloc+5
	}
}
*/
////////////////////////////////////////////////////////////////////////////////


//void* _injectcode(DWORD address)
DWORD _injectcode(void* function)
{
	DWORD address = reinterpret_cast<DWORD>(function);
	debug_out(_T("address=%08X"), address);

	DWORD region = 0;
	typedef LPVOID (WINAPI *LPNTALLOCVM)(HANDLE, PVOID, ULONG, PULONG, ULONG, ULONG);
	BYTE* p = reinterpret_cast<BYTE*>(reinterpret_cast<LPNTALLOCVM>(valloc)(::GetCurrentProcess(), 0, 10, &region, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
//	BYTE* p = reinterpret_cast<BYTE*>(_valloc(NULL, 10, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	debug_out(_T("commit=%08X"), p);

	// 10000000	8BFF		; mov edi,edi
	// 10000002	55			; push ebp
	// 10000003	8BEC		; mov ebp,esp
	// 10000005 E9XXXXXXXX	; jmp XXXXXXXX	; XXXXXXXX=jumpaddress-10000005-5

	BYTE prologue[] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC };
	::CopyMemory(p, prologue, 5);
	
	address = address + 5;
	debug_out(_T("jump=%08X"), address);

	*reinterpret_cast<BYTE*>(p+5) = 0xE9;
	*reinterpret_cast<DWORD*>(p+6) = address;

	return reinterpret_cast<DWORD>(p);
}


void _apihook()
{
	APIHook* api = new APIHook;

//	u32_gwtpid = reinterpret_cast<DWORD>(api->HookAPICall(_T("USER32.dll"), _T("GetWindowThreadProcessId"), _gwtpi)) + 5;
//	k32_op = reinterpret_cast<DWORD>(api->HookAPICall(_T("KERNEL32.dll"), _T("OpenProcess"), _op)) + 5;
//	k32_rpm = reinterpret_cast<DWORD>(api->HookAPICall(_T("KERNEL32.dll"), _T("ReadProcessMemory"), _rpm)) + 5;

	u32_gwtpid = _injectcode(api->HookAPICall(_T("USER32.dll"), _T("GetWindowThreadProcessId"), _gwtpi));
	k32_op = _injectcode(api->HookAPICall(_T("KERNEL32.dll"), _T("OpenProcess"), _op));
	k32_rpm = _injectcode(api->HookAPICall(_T("KERNEL32.dll"), _T("ReadProcessMemory"), _rpm));

	delete api;
}


void _initialize()
{
	//valloc = reinterpret_cast<DWORD>(::GetProcAddress(::GetModuleHandle(_T("KERNEL32.dll")), _T("VirtualAlloc")));
	//debug_out(_T("::VirtualAlloc=%08X"), valloc);

	valloc = ::GetProcAddress(::GetModuleHandle(_T("ntdll.dll")), _T("NtAllocateVirtualMemory"));
	debug_out(_T("::NtAllocateVirtualMemory=%08X"), valloc);
	
	_apihook();
}


BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				::DisableThreadLibraryCalls(hModule);
				_initialize();
			}
			break;

		case DLL_PROCESS_DETACH:
			break;

		default:
			__assume(0);
	}

	return TRUE;
}
