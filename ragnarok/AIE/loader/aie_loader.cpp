#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>

const int codelength = 48 + MAX_PATH;
const TCHAR* inject_modulename = _T("aie_core.dll");


bool _inject(HANDLE process, HANDLE thread, const TCHAR* basepath)
{
//	const BYTE RET264[4] = { 0x61, 0xC2, 0x08, 0x01 };
	BYTE* code = 0;

	try
	{
		CONTEXT context;
		context.ContextFlags = CONTEXT_CONTROL|CONTEXT_INTEGER;

		if (::GetThreadContext(thread, &context) == FALSE)
			throw 1;

		code = new BYTE[];
		::ZeroMemory(code, codelength);

		context.Esp -= (codelength);
		*reinterpret_cast<DWORD*>(code) = context.Esp + 44;
		*reinterpret_cast<DWORD*>(code+4) = context.Esp + 48;
		*reinterpret_cast<DWORD*>(code+8) = context.Edi;
		*reinterpret_cast<DWORD*>(code+12) = context.Esi;
		*reinterpret_cast<DWORD*>(code+16) = context.Ebp;
		*reinterpret_cast<DWORD*>(code+20) = context.Esp + codelength;
		*reinterpret_cast<DWORD*>(code+24) = context.Ebx;
		*reinterpret_cast<DWORD*>(code+28) = context.Edx;
		*reinterpret_cast<DWORD*>(code+32) = context.Ecx;
		*reinterpret_cast<DWORD*>(code+36) = context.Eax;
		*reinterpret_cast<DWORD*>(code+40) = context.Eip;
//		::CopyMemory(code+44, RET264, 4);
		*reinterpret_cast<BYTE*>(code+44) = 0x61;
		*reinterpret_cast<BYTE*>(code+45) = 0xC2;
		*reinterpret_cast<BYTE*>(code+46) = 0x08;
		*reinterpret_cast<BYTE*>(code+47) = 0x01;
		::CopyMemory(code+48, inject_modulename, ::lstrlen(inject_modulename));

		context.Eip = reinterpret_cast<DWORD>(::GetProcAddress(::GetModuleHandle(_T("kernel32")), _T("LoadLibraryA")));
		if (context.Eip == 0)
			throw 2;

		DWORD written;
		if (::WriteProcessMemory(process, reinterpret_cast<void*>(context.Esp), code, codelength, &written) == FALSE)
			throw 3;

		if (::SetThreadContext(thread, &context) == FALSE)
			throw 4;

		delete [] code;
	}
	catch (int error)
	{
		if (code != 0)
			delete [] code;

		return false;
	}

	return true;
}


int main(int argc, char* argv[])
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if (::CreateProcess(_T("AutoImo.exe"), NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi) == FALSE)
		return -1;

	_inject(pi.hProcess, pi.hThread, NULL);

	::ResumeThread(pi.hThread);
	::CloseHandle(pi.hThread);
	::WaitForInputIdle(pi.hProcess, INFINITE);
	::CloseHandle(pi.hProcess);

	return 0;
}




#pragma comment(lib, "user32")
#pragma comment(lib, "Shlwapi")
