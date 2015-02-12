#include <windows.h>
#include <process.h>
#include <tchar.h>
#include "dinput/di_core.h"


#include "debugout.hpp"
CDebugOut* core = 0;

#include "injectapi.hpp"

////////////////////////////////////////////////////////////////////////////////


unsigned int __stdcall moduleStartup(void* parameter)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////


uintptr_t core_thread = 0;


void moduleInitialize(HINSTANCE hModule)
{
	core = new CDebugOut(CDebugOut::DEBUGOUT_FILE);
	core->debugout(_T("%s"), ::GetCommandLine());

	unsigned int thread_id;
	core_thread = _beginthreadex(NULL, 0, &moduleStartup, reinterpret_cast<void *>(hModule), 0, &thread_id);
}


void moduleFinalize()
{
	::CloseHandle(reinterpret_cast<HANDLE>(core_thread));

	delete core;
}

////////////////////////////////////////////////////////////////////////////////


HMODULE hMaster = NULL;


BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				if ((hMaster = LoadOriginalLibrary(_T("dinput.dll"))) == NULL)
					return FALSE;

				::DisableThreadLibraryCalls(hModule);
				moduleInitialize(hModule);
			}
			break;

		case DLL_PROCESS_DETACH:
			{
				moduleFinalize();
				::FreeLibrary(hMaster);
			}
			break;

		default:
			__assume(0);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////


#pragma comment(lib, "shlwapi")
