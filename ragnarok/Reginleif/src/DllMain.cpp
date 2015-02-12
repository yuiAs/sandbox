#include <windows.h>
#include "WinAdvApi.h"
#include "Module.h"


VOID DllAttach(HANDLE Module)
{
    DbgPrintW(_L("DLL_PROCESS_ATTACH Module=%08X Tick=%08X\n"), Module, GetTickCount());

    DisableThreadLibraryCalls(reinterpret_cast<HMODULE>(Module));
#ifdef _WINSDK_BUILD_
    InitNativeApi();
#endif
    InitModule(Module);
}

VOID DllDetach(HANDLE Module)
{
    CleanupModule();

    DbgPrintW(_L("DLL_PROCESS_DETACH Module=%08X Tick=%08X\n"), Module, GetTickCount());
}

INT __stdcall DllMain(HANDLE Module, ULONG Reason, PVOID Reserved)
{
	switch (Reason)
	{
		case DLL_PROCESS_ATTACH:
			DllAttach(Module);
			break;
		case DLL_PROCESS_DETACH:
			DllDetach(Module);
			break;
	}

	return TRUE;
}
