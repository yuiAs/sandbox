#include "net/net.h"
#include "client.h"
#include "dbgprint.h"
#include <process.h>


Client* cl = NULL;
uintptr_t wakeup_thread = 0;

////////////////////////////////////////////////////////////////////////////////


unsigned int __stdcall wakeup(void* parameter)
{
	HANDLE parent_thread = reinterpret_cast<HANDLE>(parameter);
	
	if (::GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{
		cl->unprotect_sakray();
		::WaitForInputIdle(::GetCurrentProcess(), INFINITE);
	}
	else
	{
		::WaitForInputIdle(::GetCurrentProcess(), INFINITE);
		::SuspendThread(parent_thread);
	}

	cl->analysis();
	cl->hook();
	net::init(cl);

	::ResumeThread(parent_thread);
	::CloseHandle(parent_thread);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


void out_filetimestamp()
{
	HANDLE hFile = ::CreateFile(_T("./ijl15.dll"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FILETIME lf;
		::GetFileTime(hFile, NULL, NULL, &lf);
		::CloseHandle(hFile);

		SYSTEMTIME ls;
		::FileTimeToSystemTime(&lf, &ls);

		dbgprintf(0, _T("ijl15.dll %04d-%02d-%02d %02d:%02d:%02d\n"), ls.wYear, ls.wMonth, ls.wDay, ls.wHour+9, ls.wMinute, ls.wSecond);
	}
}

////////////////////////////////////////////////////////////////////////////////


void dll_process_attach(HINSTANCE instance)
{
#ifdef _DEBUG
	//DbgPrint::init(DbgPrint::DOUT_FILE);

	int ot = ::GetPrivateProfileInt(_T("debug"), _T("output"), DbgPrint::DOUT_API, _T("./ijl15.ini"));
	DbgPrint::init(ot);
	out_filetimestamp();
#endif

	cl = new Client;
	cl->init();

	HANDLE _th = ::GetCurrentThread();
	HANDLE _ps = ::GetCurrentProcess();
	::DuplicateHandle(_ps, _th, _ps, &_th, NULL, FALSE, DUPLICATE_SAME_ACCESS);

	unsigned int dummy;
	wakeup_thread = _beginthreadex(NULL, 0, &wakeup, reinterpret_cast<void*>(_th), 0, &dummy);

	dbgprintf(0, "current=%08X wakeup=%08X\n", ::GetCurrentThreadId(), dummy);
}


void dll_process_detach(HINSTANCE instance)
{
	net::destroy();

	if (cl)
		delete cl;

#ifdef _DEBUG
	DbgPrint::fin();
#endif
}

////////////////////////////////////////////////////////////////////////////////


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

		case DLL_PROCESS_DETACH:
			{
				dll_process_detach(hModule);
			}
			break;

		default:
			__assume(0);
	}

	return TRUE;
}
