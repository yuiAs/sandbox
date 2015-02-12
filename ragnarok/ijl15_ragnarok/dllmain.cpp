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
			pe->copyMemory(reinterpret_cast<void*>(address+2), &mod, sizeof(BYTE));
		}
	}
}


void set_cmdopt(int cmd, int flag)
{
	DWORD jmptbl = conf->get_addr(ADDR_CMDJMPTBL);
	DWORD line = *reinterpret_cast<DWORD*>(cmd*4+jmptbl);
	DWORD address = 0;

	switch (*reinterpret_cast<BYTE*>(line))
	{
		case 0x8B:	// mov ecx	; 8B0D
			address = *reinterpret_cast<DWORD*>(line+2);
			break;

		case 0xA0:	// mov al	; A0
		case 0xA1:	// mov eax	; A1
			address = *reinterpret_cast<DWORD*>(line+1);
			break;

		default:
			break;
	}

	if (address)
	{
		*reinterpret_cast<DWORD*>(address) = static_cast<DWORD>(flag);
		DbgPrint(_T("setcmdopt=%08X flag=%d address=%08X"), cmd, flag, address);
	}
}

////////////////////////////////////////////////////////////////////////////////


uintptr_t core_thread = 0;


unsigned int __stdcall moduleStartup(void* parameter)
{
	if (::GetAsyncKeyState(VK_SHIFT) & 0x8000)
	//if (conf->get_bool("extention", "sakray"))
		fix_checkcode();

	pe->waitASProtect();
	::Sleep(1);	// à”ê}ìIÇ…switchÇãNÇ±Ç≥ÇπÇÈ
	pe->analysisFF25();
	hook::start();

	using namespace autoanalyser;
	addr_gamebase();
	addr_actorbase();
	addr_aid();
	addr_xml_address();
	addr_vdtvalue();
	addr_charname();

	::WaitForInputIdle(::GetCurrentProcess(), INFINITE);

	addr_netbase();
	addr_recvsend();
	netcode::start();

	if (int value = conf->get_int("extention", "vdt_value"))
	{
		float val = static_cast<float>(value);
		DWORD address = conf->get_addr(ADDR_VDT);
		pe->copyMemory(reinterpret_cast<void*>(address), &val, sizeof(float));
	}

	{
		BYTE total = static_cast<BYTE>(conf->get_int("command_conf", "step"));
		if (total == 0)
			total = (::GetAsyncKeyState(VK_SHIFT)&0x8000) ? 0xA6 : 0xA5;

		addr_cmdjmptable(total);

		set_cmdopt(0x86, conf->get_int("command", "cmd_battlemode"));
		set_cmdopt(0x69, conf->get_int("command", "cmd_noshift"));
		set_cmdopt(0x6C, conf->get_int("command", "cmd_noctrl"));
		set_cmdopt(0x8C, conf->get_int("command", "cmd_window"));
		set_cmdopt(0x70, conf->get_int("command", "cmd_skillfail"));
		set_cmdopt(0x87, conf->get_int("command", "cmd_notrade"));
		set_cmdopt(0x6A, conf->get_int("command", "cmd_aura"));
	}

	pe->clearFF25();

	return 0;
}


void moduleInitialize(HINSTANCE hModule)
{
	// Ç∆ÇËÇ†Ç¶Ç∏class instanceÇæÇØÇÕêÊÇ…ê∂ê¨

	pe = new PEImage(::GetModuleHandle(NULL));
	if (conf = new DLLConf)
        conf->load("./ds.ini");

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

				// cuurentÇ…ds.luaÇ™Ç†ÇÈèÍçáÇÃÇ›óLå¯

				if (::GetFileAttributes(_T("./ds.ini")) != -1)
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
