// MayQueen.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"


//LPTSTR lpRoAddrIni = NULL;
LPVOID lpfnGetPPInt=NULL, lpfnGetPPStr=NULL;

////////////////////////////////////////////////////////////////////////////////


bool CheckFileName(LPCTSTR lpFileName, LPCTSTR lpFileSpec)
{
	bool bResult = false;
	LPTSTR lpBuffer = new TCHAR[MAX_PATH];

	::lstrcpy(lpBuffer, lpFileName);
	::PathStripPath(const_cast<LPTSTR>(lpBuffer));

	if (lstrcmpi(lpBuffer, lpFileSpec) == 0)
		bResult = true;

	delete [] lpBuffer;
	return bResult;
}


////////////////////////////////////////////////////////////////////////////////


UINT WINAPI _GetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
{
	typedef (WINAPI *lpGetPPInt)(LPCTSTR, LPCTSTR, INT, LPCTSTR);

	if (CheckFileName(lpFileName, _T("RagAddress.ini")))
		return reinterpret_cast<lpGetPPInt>(lpfnGetPPInt)(lpAppName, lpKeyName, nDefault, _T("./RoAddr.ini"));

	return reinterpret_cast<lpGetPPInt>(lpfnGetPPInt)(lpAppName, lpKeyName, nDefault, lpFileName);
}


BOOL WINAPI _GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	if (CheckFileName(lpFileName, _T("RagAddress.ini")) == false)
	{
		typedef (WINAPI *lpGetPPStr)(LPCTSTR, LPCTSTR, LPCTSTR, LPTSTR, DWORD, LPCTSTR);
		return reinterpret_cast<lpGetPPStr>(lpfnGetPPStr)(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
	}

	LPTSTR lpNewKeyName = new TCHAR[16];

	if (lstrcmpi(lpKeyName, _T("Exp")) == 0)
		lstrcpy(lpNewKeyName, _T("BaseExp"));
	else if (lstrcmpi(lpKeyName, _T("MaxExp")) == 0)
		lstrcpy(lpNewKeyName, _T("BaseExpNext"));
	else if (lstrcmpi(lpKeyName, _T("Job")) == 0)
		lstrcpy(lpNewKeyName, _T("JobExp"));
	else if (lstrcmpi(lpKeyName, _T("MaxJob")) == 0)
		lstrcpy(lpNewKeyName, _T("JobExpNext"));
	else if (lstrcmpi(lpKeyName, _T("MaxWeight")) == 0)
		lstrcpy(lpNewKeyName, _T("WeightMax"));
	else if (lstrcmpi(lpKeyName, _T("Name")) == 0)
		lstrcpy(lpNewKeyName, _T("CharName"));
	else
		lstrcpy(lpNewKeyName, lpKeyName);

	typedef (WINAPI *lpGetPPInt)(LPCTSTR, LPCTSTR, INT, LPCTSTR);
    wsprintf(lpReturnedString, _T("%08X"), reinterpret_cast<lpGetPPInt>(lpfnGetPPInt)(_T("RagAddress"), lpNewKeyName, 0, _T("./RoAddr.ini")));

	delete [] lpNewKeyName;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////


BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				// disables the DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications
				DisableThreadLibraryCalls(reinterpret_cast<HMODULE>(hModule));
/*
				lpRoAddrIni = new TCHAR[MAX_PATH];
				GetModuleFileName(NULL, lpRoAddrIni, MAX_PATH);
				PathReplaceFileSpec(lpRoAddrIni, _T("RoAddr.ini"));
*/
				APIHook* api = new APIHook;
				lpfnGetPPStr = api->HookAPICall("Kernel32.dll", "GetPrivateProfileStringA", _GetPrivateProfileString, 0);
				lpfnGetPPInt = api->HookAPICall("Kernel32.dll", "GetPrivateProfileIntA", _GetPrivateProfileInt, 0);
				delete api;
			}
			break;

		case DLL_PROCESS_DETACH:
			{
//				delete [] lpRoAddrIni;
			}
			break;
	}

    return TRUE;
}

