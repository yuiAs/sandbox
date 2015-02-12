#ifndef INJECTAPI_HPP
#define INJECTAPI_HPP

#include <windows.h>

#pragma warning(disable: 4311)
#pragma warning(disable: 4312)


class CInjectAPI
{

private:

	enum { ORDIAL_NO_USE=0xFFFFFFFF };

private:

	DWORD _master;

protected:

	template<typename T>
	inline T add_cast(DWORD base, DWORD add) const { return reinterpret_cast<T>(base+add); }

public:

	CInjectAPI() { _master = reinterpret_cast<DWORD>(::GetModuleHandle(NULL)); }
	~CInjectAPI();

private:

	PIMAGE_NT_HEADERS _GetPEHeader(DWORD module)
	{
		if (reinterpret_cast<PIMAGE_DOS_HEADER>(module)->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		PIMAGE_NT_HEADERS header = reinterpret_cast<PIMAGE_NT_HEADERS>(_master+reinterpret_cast<PIMAGE_DOS_HEADER>(module)->e_lfanew);
		if (header->Signature == IMAGE_NT_SIGNATURE)
			return header;

        return 0;
	}

	PIMAGE_IMPORT_DESCRIPTOR _GetImportDescriptor(const char* moduleName)
	{
		PIMAGE_NT_HEADERS header = _GetPEHeader(_master);
		if (header == 0)
			return 0;

		// http://spiff.tripnet.se/~iczelion/pe-tut6.html

		DWORD rva = header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (rva == 0)
			return 0;

		PIMAGE_IMPORT_DESCRIPTOR desc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(_master+rva);
		while (desc->FirstThunk != 0)
		{
			const char* name = reinterpret_cast<const char*>(_master+desc->Name);
			if (::lstrcmpi(moduleName, name) == 0)
				break;

			desc++;
		}

		if (desc->Name == 0)
			return 0;

		return desc;
	}

private:

	void* _inject(void* function, PIMAGE_THUNK_DATA iat)
	{
		MEMORY_BASIC_INFORMATION mbi;
		::VirtualQuery(iat, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
		::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect);

		void* original = reinterpret_cast<void*>(iat->u1.Function);
		iat->u1.Function = reinterpret_cast<DWORD>(function);

		DWORD d;
		::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &d);

		return original;
	}

public:

	void* inject(void* function, const char* moduleName, const char* exportName, DWORD ordial=ORDIAL_NO_USE)
	{
		PIMAGE_IMPORT_DESCRIPTOR desc = _GetImportDescriptor(moduleName);
		if (desc == 0)
			return 0;
		if (desc->OriginalFirstThunk == 0)
			return 0;

		PIMAGE_THUNK_DATA thunk_int = reinterpret_cast<PIMAGE_THUNK_DATA>(_master+desc->OriginalFirstThunk);
		PIMAGE_THUNK_DATA thunk_iat = reinterpret_cast<PIMAGE_THUNK_DATA>(_master+desc->FirstThunk);

		while (thunk_int->u1.Function != 0)
		{
			if (ordial == ORDIAL_NO_USE)
			{
				PIMAGE_IMPORT_BY_NAME iname = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(_master+thunk_int->u1.AddressOfData);
				if (::lstrcmp(exportName, reinterpret_cast<const char*>(iname->Name)) == 0)
					return _inject(function, thunk_iat);
			}
			else
			{
				if (thunk_int->u1.Ordinal == ordial)
					return _inject(function, thunk_iat);
			}

			thunk_int++;
			thunk_iat++;
		}

		return 0;
	}

};


#endif	// #ifndef INJECTAPI_HPP