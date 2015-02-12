#ifndef _APIHOOK_HPP_
#define _APIHOOK_HPP_
#include <windows.h>

#define MakePtr(cast, ptr, addValue) (cast)((DWORD)(ptr)+(DWORD)(addValue))

#pragma warning(disable: 4311)
#pragma warning(disable: 4312)

////////////////////////////////////////////////////////////////////////////////


class APIHook
{
	HMODULE hMasterModule;

public:
	APIHook() { hMasterModule = GetModuleHandle(NULL); }
	~APIHook() {}

private:

	///////////////////////////////////////////////////////////////////////////////

	// Given an HMODULE, returns a pointer to the PE header

	PIMAGE_NT_HEADERS PEHeaderFromHModule(HMODULE hModule)
	{
		PIMAGE_NT_HEADERS pExeNTHdr = NULL;
	
		__try
		{
			if (PIMAGE_DOS_HEADER(hModule)->e_magic != IMAGE_DOS_SIGNATURE)
				__leave;

			pExeNTHdr = PIMAGE_NT_HEADERS(PBYTE(hModule)+PIMAGE_DOS_HEADER(hModule)->e_lfanew);
			if (pExeNTHdr->Signature != IMAGE_NT_SIGNATURE)
				pExeNTHdr = NULL;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
		
		return pExeNTHdr;
	}

private:

	////////////////////////////////////////////////////////////////////////////

	// Get ImportDescriptor

	PIMAGE_IMPORT_DESCRIPTOR GetNamedImportDescriptor(LPCTSTR lpImportModule)
	{
		PIMAGE_NT_HEADERS pExeNTHdr = PEHeaderFromHModule(hMasterModule);
		if (!pExeNTHdr)
			return NULL;

		DWORD importRVA = pExeNTHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (!importRVA)
			return NULL;

		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, hMasterModule, importRVA);

		while (pImportDesc->FirstThunk)
		{
			PSTR pszImportModuleName = MakePtr(PSTR, hMasterModule, pImportDesc->Name);
			if (lstrcmpi(pszImportModuleName, lpImportModule) == 0)
				break;
			pImportDesc++;
		}

		if (pImportDesc->Name == NULL)
			return NULL;

		return pImportDesc;
	}

private:

	////////////////////////////////////////////////////////////////////////////

	// Hook at OrdinalNumber

	LPVOID _HookAPICallOrdinal(LPCTSTR lpImportModule, DWORD dwOrdinal, LPVOID lpfnModFunc)
	{
		OSVERSIONINFO osvi; 
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		GetVersionEx(&osvi);

		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = GetNamedImportDescriptor(lpImportModule);
		if (pImportDesc == NULL)
			return NULL;
		if (pImportDesc->OriginalFirstThunk == 0)
			return NULL;

		PIMAGE_THUNK_DATA pINT = MakePtr(PIMAGE_THUNK_DATA, hMasterModule, pImportDesc->OriginalFirstThunk);
		PIMAGE_THUNK_DATA pIAT = MakePtr(PIMAGE_THUNK_DATA, hMasterModule, pImportDesc->FirstThunk);

		LPVOID pfnOriginal = NULL;
		while (pINT->u1.Function)
		{
			if (IMAGE_ORDINAL(pINT->u1.Ordinal) == dwOrdinal)
			{
				MEMORY_BASIC_INFORMATION mbi_thunk;
				VirtualQuery(pIAT, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));
				VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, PAGE_READWRITE, &mbi_thunk.Protect);

				pfnOriginal = reinterpret_cast<LPVOID>(pIAT->u1.Function);

				if (IsBadWritePtr(reinterpret_cast<PVOID>(pIAT->u1.Function), 1))
					pIAT->u1.Function = reinterpret_cast<DWORD>(lpfnModFunc);
				else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
				{
					if (pIAT->u1.Function > 0x80000000)
						pIAT->u1.Function = reinterpret_cast<DWORD>(lpfnModFunc);
				}

				DWORD dwOldProtect;
				VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, mbi_thunk.Protect, &dwOldProtect);

				break;
			}

			pINT++;
			pIAT++;
		}

		return pfnOriginal;
	}


	// Hook at FunctionName

	LPVOID _HookAPICall(LPCTSTR lpImportModule, LPCTSTR lpFuncName, LPVOID lpfnModFunc)
	{
		OSVERSIONINFO osvi; 
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		GetVersionEx(&osvi);

		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = GetNamedImportDescriptor(lpImportModule);
		if (pImportDesc == NULL)
			return NULL;
		if (pImportDesc->OriginalFirstThunk == 0)
			return NULL;

		PIMAGE_THUNK_DATA pINT = MakePtr(PIMAGE_THUNK_DATA, hMasterModule, pImportDesc->OriginalFirstThunk);
		PIMAGE_THUNK_DATA pIAT = MakePtr(PIMAGE_THUNK_DATA, hMasterModule, pImportDesc->FirstThunk);

		LPVOID pfnOriginal = NULL;

		while (pINT->u1.Function)
		{
			PIMAGE_IMPORT_BY_NAME pByName = MakePtr(PIMAGE_IMPORT_BY_NAME, hMasterModule, pINT->u1.AddressOfData);
			if (*pByName->Name == NULL)
				continue;

			if (lstrcmpi(lpFuncName, reinterpret_cast<LPCTSTR>(pByName->Name)) == 0)
			{
				MEMORY_BASIC_INFORMATION mbi_thunk;
				VirtualQuery(pIAT, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));
				VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, PAGE_READWRITE, &mbi_thunk.Protect);

				pfnOriginal = reinterpret_cast<LPVOID>(pIAT->u1.Function);

				if (IsBadWritePtr(reinterpret_cast<PVOID>(pIAT->u1.Function), 1))
					pIAT->u1.Function = reinterpret_cast<DWORD>(lpfnModFunc);
				else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
				{
					if (pIAT->u1.Function > 0x80000000)
						pIAT->u1.Function = reinterpret_cast<DWORD>(lpfnModFunc);
				}

				DWORD dwOldProtect;
				VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, mbi_thunk.Protect, &dwOldProtect);

				break;
			}

			pINT++;
			pIAT++;
		}

		return pfnOriginal;
	}

	////////////////////////////////////////////////////////////////////////////

	// Force Hook at FunctionName

	LPVOID ForceHookAPICall(DWORD dwAddress, LPCTSTR lpImportModule, LPCTSTR lpFuncName, LPVOID llpfnModFunc)
	{
		LPVOID lpfnOriginal = GetProcAddress(GetModuleHandle(lpImportModule), lpFuncName);
		DWORD dwValue = 0;

		CopyMemory(&dwValue, reinterpret_cast<LPCVOID>(dwAddress), sizeof(DWORD));
		if (dwValue == 0)
			return NULL;

		DWORD dwOldProtect;
		VirtualProtect(reinterpret_cast<LPVOID>(dwAddress), sizeof(DWORD), PAGE_READWRITE, &dwOldProtect);
		CopyMemory(reinterpret_cast<LPVOID>(dwAddress), &llpfnModFunc, sizeof(DWORD));
		VirtualProtect(reinterpret_cast<LPVOID>(dwAddress), sizeof(DWORD), dwOldProtect, &dwOldProtect);

		if (dwValue == reinterpret_cast<DWORD>(lpfnOriginal))
			return lpfnOriginal;
		else
			return reinterpret_cast<LPVOID>(dwValue);

		return NULL;
	}

	////////////////////////////////////////////////////////////////////////////

	// Interface

public:

	LPVOID HookAPICall(LPCTSTR lpImportModule, LPCTSTR lpFuncName, LPVOID lpfnModFunc, DWORD dwAddress)
	{
		return _HookAPICall(lpImportModule, lpFuncName, lpfnModFunc);
		//return ForceHookAPICall(dwAddress, lpImportModule, lpFuncName, lpfnModFunc);
	}


	LPVOID HookAPICallOrdinal(LPCTSTR lpImportModule, DWORD dwOrdinal, LPVOID lpfnModFunc, DWORD dwAddress)
	{
		return _HookAPICallOrdinal(lpImportModule, dwOrdinal, lpfnModFunc);
		//return NULL;
	}

};

#endif	// #ifndef _APIHOOK_HPP_