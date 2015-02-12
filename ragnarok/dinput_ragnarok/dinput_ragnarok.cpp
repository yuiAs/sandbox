#include <winsock2.h>
#include <windows.h>
#include <tchar.h>

#include "dinput/di_core.h"
#include "Ragnarok.hpp"
#include "AttachMemory.hpp"

#include "clientHook.hpp"
#include "packet/Packet.hpp"
#include "shortcut/Shortcut.hpp"


CRagnarok* core = 0;

CClientHook* clhook = 0;
CPacket* packet = 0;
CShortcut* shortcut = 0;

////////////////////////////////////////////////////////////////////////////////


unsigned int __stdcall _startup(void* parameter)
{
	CAttachMemory* ci = new CAttachMemory;


	::WaitForInputIdle(::GetCurrentProcess(), INFINITE);

	while (clhook->getWnd() == 0) ::Sleep(10);	// 10ms‚Í‚È‚¢‚Æ‘¼‚ÌƒXƒŒƒbƒh‚ª‰ñ‚è‚«‚ç‚È‚¢
	core->setWnd(clhook->getWnd());


	// pushtext_function/gamebase address

	{
		// 68XXXXXXXX	push XXXXXXXX		; "Guild Emblem On"
		// 6A01			push 00000001
		// B9YYYYYYYY	mov ecx, YYYYYYYY	; gamebase
		// E8ZZZZZZZZ	call ZZZZZZZZ		; pushtext_function

		core->output(_T("[SECT] pushtext_function/gamebase"));

		DWORD addr_string = ci->SearchData("Guild Emblem On");
		core->output(_T("[STRING] Guild Emblem On=%08X"), addr_string);

		BYTE code[8];
		code[0] = 0x68;
		*reinterpret_cast<DWORD*>(code+1) = addr_string;
		code[5] = 0x6A;
		code[6] = 0x01;
		code[7] = 0xB9;

		DWORD addr_gamebase = ci->SearchCode(code, 8, 8);
		core->setAddr(AD_GAMEBASE, addr_gamebase);
		core->output(_T("gamebase=%08X"), addr_gamebase);


		DWORD shift_pushtext = ci->SearchCode(code, 8, 13);

//		DWORD addr_pushtext = ci->GetCallAddress(shift_pushtext);

		BYTE code2[10];
		code2[0] = 0xB9;
		*reinterpret_cast<DWORD*>(code2+1) = addr_gamebase;
		code2[5] = 0xE8;
		*reinterpret_cast<DWORD*>(code2+6) = shift_pushtext;

		DWORD addr_pushtext = ci->GetCodeAddress(code2, 10) + 10 + shift_pushtext;

		core->setAddr(AD_CALL_PUSHTEXT, addr_pushtext);
		core->output(_T("pushtext_function=%08X (%08X)"), addr_pushtext, shift_pushtext);
	}

	// actorbase address

	{
		// 68XXXXXXXX	push XXXXXXXX		; "nothing"
		// B9YYYYYYYY	mov ecx, YYYYYYYY	; actorbase

		core->output(_T("[SECT] actorbase"));

		DWORD addr_string = ci->SearchData("nothing");
		core->output(_T("[STRING] nothing=%08X"), addr_string);

		BYTE code[6];
		code[0] = 0x68;
		*reinterpret_cast<DWORD*>(code+1) = addr_string;
		code[5] = 0xB9;

		DWORD addr_actorbase = ci->SearchCode(code, 6, 6);
		core->setAddr(AD_ACTORBASE, addr_actorbase);
		core->output(_T("actorbase=%08X"), addr_actorbase);
	}

	// account_server_ip address

	{
		// C70584876700D88D6700	mov dword ptr [YYYYYYYY], XXXXXXXX

		core->output(_T("[SECT] account_server_ip"));

		DWORD addr_string = ci->SearchData("192.168.0.100");
		core->output(_T("[STRING] 192.168.0.100=%08X"), addr_string);

		BYTE code[4];
		*reinterpret_cast<DWORD*>(code) = addr_string;

		DWORD addr_acsvip = ci->SearchCode(code, 4, -4);
		core->setAddr(AD_ACCSVIP, addr_acsvip);
		core->output(_T("account_server_ip=%08X"), addr_acsvip);
	}

	// skill_id (string)
	
	{
		// 8B0DYYYYYYYY	mov ecx, dword ptr [YYYYYYYY]	; address
		// C701XXXXXXXX	mov dword ptr [ecx], XXXXXXXX	; "Zero Skill"

		core->output(_T("[SECT] skill_id (string)"));

		DWORD addr_string = ci->SearchData("Zero Skill");
		core->output(_T("[STRING] Zero Skill=%08X"), addr_string);

		BYTE code[6];
		code[0] = 0xC7;
		code[1] = 0x01;
		*reinterpret_cast<DWORD*>(code+2) = addr_string;

		DWORD addr_skill_id = ci->SearchCode(code, 6, -4);
		core->setAddr(AD_SKILLID, addr_skill_id);
		core->output(_T("skill_id=%08X"), addr_skill_id);

		core->cl_pushSkillID(0x01EC, _T("ITM_DARKWEAPON"));
	}

	// npc_id (string)

	{
		// BBXXXXXXXX	mov ebx, XXXXXXXX				; "1_ETC_01"
		// 8DB7YYYYYYYY	lea esi, dword ptr[edi+YYYYYYYY]; address = actorbase + YYYYYYYY

		core->output(_T("[SECT] npc_id (string)"));
		DWORD addr_actorbase = core->getAddr(AD_ACTORBASE);

		DWORD addr_string = ci->SearchData("1_ETC_01");
		core->output(_T("[STRING] 1_ETC_01=%08X"), addr_string);

		BYTE code[4];
		*reinterpret_cast<DWORD*>(code) = addr_string;

		DWORD addr_npc_id = ci->SearchCode(code, 4, 6);
		core->setAddr(AD_NPCID, addr_npc_id+addr_actorbase);
		core->output(_T("npc_id=%08X (%08X)"), addr_npc_id+addr_actorbase, addr_npc_id);
	}

	// view_distance_top_value address

	if (float val = static_cast<float>(::GetPrivateProfileInt(_T("system"), _T("vdt_value"), 0, _T("./dinput.ini"))))
	{
		// 68YYYYYYYY	push YYYYYYYY		; g_outdoorViewDistance
		// 6A04			push 00000004		; sizeof(DWORD)
		// 6A00			push 00000000
		// 68XXXXXXXX	push XXXXXXXX		; "g_outdoorViewDistance"

		core->output(_T("[SECT] view_distance_top_value"));

		DWORD addr_string = ci->SearchData("g_outdoorViewDistance");
		core->output(_T("[STRING] g_outdoorViewDistance=%08X"), addr_string);

		BYTE code[9];
		code[0] = 0x6A;
		code[1] = 0x04;
		code[2] = 0x6A;
		code[3] = 0x00;
		code[4] = 0x68;
		*reinterpret_cast<DWORD*>(code+5) = addr_string;

		DWORD addr_vdt = ci->SearchCode(code, 9, -4);
		if (addr_vdt != 0)
		{
			core->setAddr(AD_VDT, addr_vdt-0x1C);	// -0x1C‚É‚Â‚¢‚Ä‚Í—ª
			core->output(_T("view_distance_top_value=%08X"), addr_vdt-0x1C);

			ci->MemoryPatch(addr_vdt-0x1C, &val, sizeof(float));
		}
	}

	// shortcut

	if (shortcut != 0)
	{
		core->output(_T("[SECT] shortcut"));

		// showscwnd_function/scwndflag

		// 6A00			push 00000000
		// 6A24			push 00000024
		// B9XXXXXXXX	mov ecx, XXXXXXXX	; gamebase
		// E8YYYYYYYY	call YYYYYYYY		; show_shortcutwnd_function

		DWORD addr_gamebase = core->getAddr(AD_GAMEBASE);

		BYTE code3[10];
		code3[0] = 0x6A;
		code3[1] = 0x00;
		code3[2] = 0x6A;
		code3[3] = 0x24;
		code3[4] = 0xB9;
		*reinterpret_cast<DWORD*>(code3+5) = addr_gamebase;
		code3[9] = 0xE8;

		DWORD shift_showscwnd = ci->SearchCode(code3, 10, 10);

		DWORD addr_showscwnd = ci->GetCallAddress(shift_showscwnd);
		core->setAddr(AD_CALL_SHSCWND, addr_showscwnd);
		core->output(_T("showscwnd_function=%08X (%08X)"), addr_showscwnd, shift_showscwnd);

		// 8D93XXXXXXXX	lea edx, dword ptr [ebx+XXXXXXXX]	; scwndflag_shift
		// 6A04				push 00000004
		// 52				push edx
		// 6A00				push 00000000
		// 68YYYYYYYY		push YYYYYYYY					; "SHORTCUTWNDINFO.SHOW"

		DWORD addr_string = ci->SearchData("SHORTCUTWNDINFO.SHOW");
		core->output(_T("[STRING] SHORTCUTWNDINFO.SHOW=%08X"), addr_string);

		BYTE code5[12];
		code5[0] = 0x6A;
		code5[1] = 0x04;
		code5[2] = 0x52;
		code5[3] = 0x6A;
		code5[4] = 0x04;
		code5[5] = 0x6A;
		code5[6] = 0x00;
		code5[7] = 0x68;
		*reinterpret_cast<DWORD*>(code5+8) = addr_string;

		DWORD shift_scwndflag = ci->SearchCode(code5, 12, -4);
		core->setAddr(AD_SCWNDFLAG, shift_scwndflag+addr_gamebase);
		core->output(_T("scwndflag=%08X (%08X)"), shift_scwndflag+addr_gamebase, shift_scwndflag);



		DWORD addr_actorbase = core->getAddr(AD_ACTORBASE);

		// SLBase

		// 899481XXXXXXXX	mov dword ptr [ecx+4*eax+XXXXXXXX], ebx	; SLBase_shift

		BYTE code1[3];
		code1[0] = 0x89;
		code1[1] = 0x94;
		code1[2] = 0x81;

		DWORD shift_slbase = ci->SearchCode(code1, 3, 3);
		core->setAddr(AD_SLBASE, shift_slbase+addr_actorbase);
		core->output(_T("SLBase=%08X (%08X)"), shift_slbase+addr_actorbase, shift_slbase);

		// SNBase

		// C1E104			shl ecx, 04
		// 6A01				push 00000001
		// 8D9C11XXXXXXXX	lea ebx, dword ptr [ecx+ebx+XXXXXXXX]	; SNBase_shift

		BYTE code2[5];
		code2[0] = 0xC1;
		code2[1] = 0xE1;
		code2[2] = 0x04;
		code2[3] = 0x6A;
		code2[4] = 0x01;

		DWORD shift_snbase = ci->SearchCode(code2, 5, 8);
		core->setAddr(AD_SNBASE, shift_snbase+addr_actorbase);
		core->output(_T("SNBase=%08X (%08X)"), shift_snbase+addr_actorbase, shift_snbase);

		// SCPage

		core->setAddr(AD_SCPAGE, shift_snbase+addr_actorbase-0x04);
		core->output(_T("SCPage=%08X"), shift_snbase+addr_actorbase-0x04);
	}


	delete ci;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

// 2005-12-05aSakexe or later

void fix_checkcode()
{
	CAttachMemory* ci = new CAttachMemory;

	// new 00050105050502030505050504
	// old 00050505050102030505050504

	core->output(_T("[SECT] fix_checkcode (2005-12-05aSakexe or later)"));

	BYTE code[7];
	code[0] = 0x02;
	code[1] = 0x03;
	code[2] = 0x05;
	code[3] = 0x05;
	code[4] = 0x05;
	code[5] = 0x05;
	code[6] = 0x04;

	DWORD addr_checkcode = ci->GetCodeAddress(code, 7) - 6;
	core->output(_T("checkcode=%08X"), addr_checkcode);
	core->output(_T("langtype=2=%02X"), *reinterpret_cast<BYTE*>(addr_checkcode+2));

	if (*reinterpret_cast<BYTE*>(addr_checkcode+2) != 0x05)
	{
		BYTE mod = 0x05;
		ci->MemoryPatch(addr_checkcode+2, &mod, sizeof(BYTE), true);
	}

	delete ci;
}

////////////////////////////////////////////////////////////////////////////////


uintptr_t core_thread = 0;


void moduleInitialize(HINSTANCE hModule)
{
	core = new CRagnarok(hModule);
	//core->output(_T("%s"), ::GetCommandLine());
	//core->output(_T("%s"), ::PathGetArgs(::GetCommandLine()));

	clhook = new CClientHook;
	packet = new CPacket;

	if (::GetPrivateProfileInt(_T("shortcut"), _T("enable"), 0, _T("./dinput.ini")))
		shortcut = new CShortcut(::GetCurrentThreadId());

	fix_checkcode();

	unsigned int thread_id;
	core_thread = _beginthreadex(NULL, 0, &_startup, reinterpret_cast<void *>(hModule), 0, &thread_id);
}


void moduleFinalize()
{
	::CloseHandle(reinterpret_cast<HANDLE>(core_thread));

	delete shortcut;
	delete packet;
	delete clhook;

	delete core;
}

////////////////////////////////////////////////////////////////////////////////


HMODULE hLibrary = NULL;


BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				if ((hLibrary = LoadOriginalLibrary()) == NULL)
					return FALSE;

				::DisableThreadLibraryCalls(hModule);
				moduleInitialize(hModule);
			}
			break;

		case DLL_PROCESS_DETACH:
			{
				moduleFinalize();
				::FreeLibrary(hLibrary);
			}
			break;

		default:
			__assume(0);
	}

	return TRUE;
}
