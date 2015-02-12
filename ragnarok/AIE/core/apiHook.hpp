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

private:
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

	// Given an HMODULE, returns a pointer to the PE header
	PIMAGE_NT_HEADERS PEHeaderFromHModule(HMODULE hModule)
	{
		PIMAGE_NT_HEADERS pNTHeader = 0;
	
		__try
		{
			if (PIMAGE_DOS_HEADER(hModule)->e_magic != IMAGE_DOS_SIGNATURE)
				__leave;

			pNTHeader = PIMAGE_NT_HEADERS(PBYTE(hModule)+PIMAGE_DOS_HEADER(hModule)->e_lfanew);
			if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
				pNTHeader = 0;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
		
		return pNTHeader;
	}

public:
	APIHook() { hMasterModule = GetModuleHandle(NULL); }
	~APIHook() {}

	LPVOID HookAPICallOrdinal(LPCTSTR lpImportModule, DWORD dwOrdinal, LPVOID pfnModFunc)
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
					pIAT->u1.Function = reinterpret_cast<DWORD>(pfnModFunc);
				else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
				{
					if (pIAT->u1.Function > 0x80000000)
						pIAT->u1.Function = reinterpret_cast<DWORD>(pfnModFunc);
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

	LPVOID HookAPICall(LPCTSTR lpImportModule, LPCTSTR lpFuncName, LPVOID pfnModFunc)
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
					pIAT->u1.Function = reinterpret_cast<DWORD>(pfnModFunc);
				else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
				{
					if (pIAT->u1.Function > 0x80000000)
						pIAT->u1.Function = reinterpret_cast<DWORD>(pfnModFunc);
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


};

#endif	// #ifndef _APIHOOK_HPP_