#include "stdafx.h"
#include <process.h>
#include "dllconf.hpp"
#include "peimage.hpp"
#include "autoanalyser.hpp"
#include "clienthook.hpp"
#include "packet/netcode.hpp"
#include "packet/packet.hpp"


PEImage* pe = 0;
DLLConf* conf = 0;
CPacket* packet = 0;

////////////////////////////////////////////////////////////////////////////////


void fix_checkcode()
{
	// new 00050105050502030505050504
	// old 00050505050102030505050504

	BYTE code[] = {
		0x02, 0x03, 0x05, 0x05, 0x05, 0x05, 0x04,
	};

	DWORD address = pe->searchCode(code, 7) - 6;
	if (::IsBadReadPtr(reinterpret_cast<const void*>(address), 13) == 0)
	{
		DbgPrint(_T("fix_checkcode address=%08X code=%02X"), address, *reinterpret_cast<BYTE*>(address+2));

		if (*reinterpret_cast<BYTE*>(address+2) != 0x05)
		{
			BYTE mod = 0x05;
			pe->attachMemory(reinterpret_cast<void*>(address+2), &mod, sizeof(BYTE));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


uintptr_t core_thread = 0;


unsigned int __stdcall moduleStartup(void* parameter)
{
	if (::GetAsyncKeyState(VK_SHIFT) & 0x8000)
		fix_checkcode();

	pe->waitASProtect();
	pe->analysisFF25();
	hook::start();

	using namespace autoanalyser;
	addr_gamebase();
	addr_actorbase();
	addr_aid();
	addr_xml_address();
	addr_vdtvalue();
	addr_netbase();

	//addr_recvsend();
	//netcode::start();
/*
	if (int value = conf->get_int("extention", "vdt_value") != -1)
	{
		DWORD address = conf->get_addr(ADDR_VDT);
		float val = static_cast<float>(value);
		pe->attachMemory(reinterpret_cast<void*>(address), &val, sizeof(float));
	}
*/
	pe->clearFF25();

	return 0;
}


void moduleInitialize(HINSTANCE hModule)
{
	// Ç∆ÇËÇ†Ç¶Ç∏class instanceÇæÇØÇÕêÊÇ…ê∂ê¨

	pe = new PEImage(::GetModuleHandle(NULL));
	if (conf = new DLLConf)
        conf->load("cpsx.lua");
	packet = new CPacket;


	unsigned int thread_id;
	core_thread = _beginthreadex(NULL, 0, &moduleStartup, reinterpret_cast<void *>(hModule), 0, &thread_id);
}


void moduleFinalize()
{
	if (packet)
		delete packet;
	if (pe)
		delete pe;
	if (conf)
		delete conf;

	::CloseHandle(reinterpret_cast<HANDLE>(core_thread));
}

////////////////////////////////////////////////////////////////////////////////


BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				::DisableThreadLibraryCalls(hModule);

				// cuurentÇ…cpsx.luaÇ™Ç†ÇÈèÍçáÇÃÇ›óLå¯

				if (::GetFileAttributes(_T("./cpsx.lua")) != -1)
					moduleInitialize(hModule);
			}
			break;

		case DLL_PROCESS_DETACH:
			{
				if (core_thread != 0)
					moduleFinalize();
			}
			break;

		default:
			__assume(0);
	}

	return TRUE;
}
