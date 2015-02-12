#include <windows.h>
#include <process.h>
#include "effect.h"

////////////////////////////////////////////////////////////////////////////////


uintptr_t wakeup_thread = 0;


unsigned int __stdcall wakeup(void* parameter)
{
	HANDLE parent_thread = reinterpret_cast<HANDLE>(parameter);
	
	::WaitForInputIdle(::GetCurrentProcess(), INFINITE);
	::SuspendThread(parent_thread);

	effect_trampoline();

	::ResumeThread(parent_thread);
	::CloseHandle(parent_thread);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


void dll_process_attach(HINSTANCE hModule)
{
	HANDLE _th = ::GetCurrentThread();
	HANDLE _ps = ::GetCurrentProcess();
	::DuplicateHandle(_ps, _th, _ps, &_th, NULL, FALSE, DUPLICATE_SAME_ACCESS);

	unsigned int dummy;
	wakeup_thread = _beginthreadex(NULL, 0, &wakeup, reinterpret_cast<void*>(_th), 0, &dummy);
}


void dll_process_detach(HINSTANCE hModule)
{
	effect_uncommit();
}

////////////////////////////////////////////////////////////////////////////////


BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				::DisableThreadLibraryCalls(hModule);
				dll_process_attach(hModule);
			}
			break;

		case DLL_PROCESS_DETACH:
			{
				dll_process_detach(hModule);
			}
			break;
	}

	return TRUE;
}
