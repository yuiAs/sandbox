#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>


FARPROC p_DirectInputCreateA;
FARPROC p_DirectInputCreateEx;
FARPROC p_DirectInputCreateW;
FARPROC p_DllCanUnloadNow;
FARPROC p_DllGetClassObject;
FARPROC p_DllRegisterServer;
FARPROC p_DllUnregisterServer;


HINSTANCE LoadOriginalLibrary(TCHAR* filename)
{
	TCHAR* dllName = new TCHAR[MAX_PATH];

	::GetSystemDirectory(dllName, MAX_PATH);
	::PathAppend(dllName, filename);

	HINSTANCE hLibrary = ::LoadLibrary(dllName);
	if (hLibrary != NULL)
	{
		p_DirectInputCreateA = ::GetProcAddress(hLibrary, _T("DirectInputCreateA"));
		p_DirectInputCreateEx = ::GetProcAddress(hLibrary, _T("DirectInputCreateEx"));
		p_DirectInputCreateW = ::GetProcAddress(hLibrary, _T("DirectInputCreateW"));
		p_DllCanUnloadNow = ::GetProcAddress(hLibrary, _T("DllCanUnloadNow"));
		p_DllGetClassObject = ::GetProcAddress(hLibrary, _T("DllGetClassObject"));
		p_DllRegisterServer = ::GetProcAddress(hLibrary, _T("DllRegisterServer"));
		p_DllUnregisterServer = ::GetProcAddress(hLibrary, _T("DllUnregisterServer"));
	}

	delete [] dllName;
	return hLibrary;
}


#pragma warning(disable: 4508)
__declspec(naked) d_DirectInputCreateA() { _asm{ jmp p_DirectInputCreateA } }
__declspec(naked) d_DirectInputCreateEx() { _asm{ jmp p_DirectInputCreateEx } }
__declspec(naked) d_DirectInputCreateW() { _asm{ jmp p_DirectInputCreateW } }
__declspec(naked) d_DllCanUnloadNow() { _asm{ jmp p_DllCanUnloadNow } }
__declspec(naked) d_DllGetClassObject() { _asm{ jmp p_DllGetClassObject } }
__declspec(naked) d_DllRegisterServer() { _asm{ jmp p_DllRegisterServer } }
__declspec(naked) d_DllUnregisterServer() { _asm{ jmp p_DllUnregisterServer } }
