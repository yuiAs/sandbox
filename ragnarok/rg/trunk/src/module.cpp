#include "intrnl.h"
#include <process.h>
#include "NTPE.h"
#include "client.h"

////////////////////////////////////////////////////////////////////////////////

EffectController* Effector = NULL;

void InitEffector()
{
	if (ULONG NPBase = g_Intrnl.Addr[IA_NPBASE])
	{
		HANDLE NPEvent = *reinterpret_cast<PHANDLE>(NPBase+0x18);
		ResetEvent(NPEvent);
	}
/*
	Effector = new EffectController;
	Effector->Init(g_Intrnl.Addr[IA_EFFTBL1], g_Intrnl.Addr[IA_EFFTBL2], g_Intrnl.Addr[IA_MINEFFECT]);
	Effector->Disabled(EC_STORMGUST);
*/
}

void DestroyEffector()
{
/*
	delete Effector;
	Effector = NULL;
*/
}

////////////////////////////////////////////////////////////////////////////////

INTRNL_TBL g_Intrnl;
uintptr_t g_CoreThread = NULL;

unsigned int __stdcall CoreThread(PVOID Parameter)
{
	WaitForInputIdle(GetCurrentProcess(), INFINITE);
	SwitchToThread();

	DbgNTPE();

	if (g_Intrnl.Threshold)
		SetDataSection(g_Intrnl.Threshold);

	Resolver();
	SakrayResolver();

	InitNetwork();
	InitEffector();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////


void LoadModulePref()
{
	RtlZeroMemory(&g_Intrnl, sizeof(INTRNL_TBL));
//	RtlZeroMemory(g_Intrnl.Addr, sizeof(g_Intrnl.Addr));

	g_Intrnl.Threshold = ConfGetVal(_CRT_WIDE("DEBUG"), _CRT_WIDE("THRESHOLD"), 0);
	g_Intrnl.DisableSend = ConfIsEnable(_CRT_WIDE("LOCAL"), _CRT_WIDE("NOSEND"));

	g_Intrnl.DisableEXMsg = ConfIsEnable(_CRT_WIDE("LOCAL"), _CRT_WIDE("EXMSG-DISABLE"));
	g_Intrnl.ClCmd[CLCMD_SH]= ConfIsEnable(_CRT_WIDE("CLCMD"), _CRT_WIDE("SH"));
	g_Intrnl.ClCmd[CLCMD_WI]= ConfIsEnable(_CRT_WIDE("CLCMD"), _CRT_WIDE("WI"));
}

void InitModule(HANDLE Module)
{
	LoadModulePref();
	g_CoreThread = _beginthreadex(NULL, 0, CoreThread, NULL, 0, NULL);
}

void DestroyModule()
{
	CloseHandle(reinterpret_cast<HANDLE>(g_CoreThread));

	DestroyNetwork();
	DestroyEffector();
}
