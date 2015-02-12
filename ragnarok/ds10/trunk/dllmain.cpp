#include "common.h"
#include <process.h>
#include "config.hpp"
#include "pe_operate.hpp"
#include "patch/client.hpp"
#include "network/network.hpp"
#include <ddraw.h>

PEOp* pe = NULL;

////////////////////////////////////////////////////////////////////////////////


uintptr_t startup_thread = 0;

unsigned int __stdcall module_startup(void* parameter)
{
	if (::GetAsyncKeyState(VK_SHIFT) & 0x8000)
		client::unprotect_sakray();
	else
	{
		::WaitForInputIdle(::GetCurrentProcess(), INFINITE);
		::SuspendThread(reinterpret_cast<HANDLE>(parameter));
	}

	config::load_config();

	pe->jmp_build();
	client::apihook();
	client::search();
	pe->jmp_clear();

	{
		using namespace client;

		DWORD netbase = client::address(AD_NET);
		DWORD recv = client::address(AD_RECV);
		DWORD send = client::address(AD_SEND);
	
		network::init(netbase, recv, send);
	}

	::ResumeThread(reinterpret_cast<HANDLE>(parameter));
	::CloseHandle(reinterpret_cast<HANDLE>(parameter));

/*
	{
//		HWND hwnd = NULL;
//		while ((hwnd=FindWindow(NULL, _T("Ragnarok")))==NULL)
//			::Sleep(1);

		DWORD address = 0x006D2DB8;
		LPDIRECTDRAW7 lpDD7 = *reinterpret_cast<LPDIRECTDRAW7*>(address+0x40);
		lpDD7->AddRef();
		dbgprintf(0, "LPDIRECTDRAW7=%08X\n", lpDD7);
		lpDD7->Release();
	}
*/

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


inline bool isFileExist(const char* filename)
{
	return ::GetFileAttributes(filename)!=0xFFFFFFFF;
}

////////////////////////////////////////////////////////////////////////////////


void module_initialize(HINSTANCE hModule)
{
	config::load("./ijl15.ini");

#ifdef _DEBUG
	{
		int ot = config::enabled("extention", "debug_log") ? DbgPrint::DOUT_FILE : DbgPrint::DOUT_API;
		DbgPrint::init(ot);
	}
#endif

#ifdef _DEBUG
	{
		HANDLE hFile = ::CreateFile("./ijl15.dll", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			FILETIME lf;
			::GetFileTime(hFile, NULL, NULL, &lf);
			::CloseHandle(hFile);

			SYSTEMTIME ls;
			::FileTimeToSystemTime(&lf, &ls);

			dbgprintf(0, "ijl15.dll %04d-%02d-%02d %02d:%02d:%02d\n", ls.wYear, ls.wMonth, ls.wDay, ls.wHour+9, ls.wMinute, ls.wSecond);
		}
	}
#endif

	pe = new PEOp(::GetModuleHandle(NULL));

	HANDLE current_thread = NULL;
	::DuplicateHandle(::GetCurrentProcess(), ::GetCurrentThread(), ::GetCurrentProcess(), &current_thread, NULL, FALSE, DUPLICATE_SAME_ACCESS);

	unsigned int thread_id;
	startup_thread = _beginthreadex(NULL, 0, &module_startup, reinterpret_cast<void*>(current_thread), 0, &thread_id);
	dbgprintf(0, "current=%08X startup=%08X\n", ::GetCurrentThreadId(), thread_id);
}


void module_finalize()
{
	SAFE_DELETE(pe);
	::CloseHandle(reinterpret_cast<HANDLE>(startup_thread));

	network::fin();
#ifdef _DEBUG
	DbgPrint::fin();
#endif

	client::apihook_cleanup();
}

////////////////////////////////////////////////////////////////////////////////


BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				::DisableThreadLibraryCalls(hModule);

				if (isFileExist("./ijl15.ini"))
					module_initialize(hModule);
			}
			break;

		case DLL_PROCESS_DETACH:
			{
				if (isFileExist("./ijl15.ini"))
					module_finalize();
			}
			break;

		default:
			__assume(0);
	}

	return TRUE;
}
