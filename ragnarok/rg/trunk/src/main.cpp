#include "intrnl.h"

////////////////////////////////////////////////////////////////////////////////


void DllAttach(HANDLE Module)
{
	// disable DLL_THREAD_ATTACH/DLL_THREAD_DETACH notifications
	DisableThreadLibraryCalls(reinterpret_cast<HMODULE>(Module));

#ifdef DBG
	int DbgLv = ConfGetVal(_CRT_WIDE("DEBUG"), _CRT_WIDE("LEVEL"), DBGINF);
	InitDebug(DbgLv);
#else
	InitDebug(DBGREL);
#endif
	DbgPrintW(DBGINF, _CRT_WIDE("INF DLL_PROCESS_ATTACH Module=%08X Tick=%08X\n"), Module, GetTickCount());

	InitModule(Module);
}

void DllDetach(HANDLE Module)
{
	DestroyModule();

	DbgPrintW(DBGINF, _CRT_WIDE("INF DLL_PROCESS_DETACH Module=%08X Tick=%08X\n"), Module, GetTickCount());
	CloseDebug();
}

////////////////////////////////////////////////////////////////////////////////


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
