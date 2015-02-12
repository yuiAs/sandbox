#include <windows.h>

////////////////////////////////////////////////////////////////////////////////


void dll_process_attach(HINSTANCE hModule)
{
	if (::GetFileAttributes(_CRT_WIDE("randgriz.dll")) != INVALID_FILE_ATTRIBUTES)
	{
		HMODULE module = ::LoadLibraryW(_CRT_WIDE("randgriz.dll"));
#ifdef _DEBUG
		if (module)
			::OutputDebugStringW(_CRT_WIDE("LOAD_SUCCESS randgriz.dll\n"));
#endif
	}
	else
	{
#ifdef _DEBUG
		::OutputDebugStringW(_CRT_WIDE("FILE_NOT_FOUND randgriz.dll\n"));
#endif
	}
}

///////////////////////////////////////////////////////////////////////////


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
	}

	return TRUE;
}
