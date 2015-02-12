#include "intrnl.h"
#include "NTPE.h"
#include "client.h"		// for PLT_ENTRY

////////////////////////////////////////////////////////////////////////////////

// IA_SCENEBASE

void Resolver_Ph1()
{
	ULONG Pos = 0;
	UCHAR Code[] = {
		0x55,								// push	ebp
		0x8B, 0xEC,							// mov	ebp, esp
		0x56,								// push	esi
		0x8B, 0xF1,							// mov	esi, ecx
		0xB9,// 0x00, 0x00, 0x00, 0x00,		// mov	ecx, imm32
	};

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return;

	ULONG SceneBase = *reinterpret_cast<ULONG*>(Pos+sizeof(Code));
	g_Intrnl.Addr[IA_SCENEBASE] = SceneBase;

	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver1 Pos=%08X+%08X\n"), Pos, sizeof(Code));
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver1 SceneBase=%08X\n"), SceneBase);
}

// IA_ZONETABLE, IP_GETNETBASE, IA_NETBASE

void Resolver_Ph2()
{
	// 52				// push		edx
	// E8********		// call		IP_GETNETBASE
	// 8BC8				// mov		ecx, eax
	// E8********		// call		rel32
	// 0FBFC0			// movsx	eax, ax
	// 83C08D			// add		eax, 0FFFFFF8Dh	// add	eax, -73
	// 3D********		// cmp		eax, imm32
	// 0F87********		// ja		rel32
	// FF2485********	// jmp		[IA_ZONETABLE+eax*4]

	ULONG Pos = 0;
	UCHAR Code[] = {
		0x0F, 0xBF, 0xC0,	// movsx	eax, ax
		0x83, 0xC0, 0x8D,	// add		eax, 0FFFFFF8Dh	// add	eax, -73
	};

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return;

	ULONG ZoneTable=0;
	ULONG FnGetNetBase=0, NetBase=0;

	if (CompareBinary<USHORT>(Pos+17, 0x24FF))				// jmp	r/m32
	{
		ZoneTable = *reinterpret_cast<ULONG*>(Pos+20);
		g_Intrnl.Addr[IA_ZONETABLE] = ZoneTable;
	}

	if (ResolveEIP(Pos-12, &FnGetNetBase))
	{
		g_Intrnl.Addr[IP_GETNETBASE] = FnGetNetBase;
		NetBase = GetNetBase();
		g_Intrnl.Addr[IA_NETBASE] = NetBase;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver2 Pos=%08X %08X %08X\n"), Pos, Pos+17, Pos-12);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver2 ZoneTable=%08X\n"), ZoneTable);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver2 GetNetBase=%08X\n"), FnGetNetBase);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver2 NetBase=%08X\n"), NetBase);
}

// IA_RECV, IA_SEND

void Resolver_Ph3()
{
	if (g_Intrnl.Addr[IA_NETBASE] == 0)
		return;

	HMODULE Ws2Handle = GetModuleHandleW(_CRT_WIDE("WS2_32.DLL"));

	ULONG NetBase = g_Intrnl.Addr[IA_NETBASE];	//GetNetBase();
	ULONG WS2Recv = reinterpret_cast<ULONG>(GetProcAddress(Ws2Handle, "recv"));
	ULONG WS2Send = reinterpret_cast<ULONG>(GetProcAddress(Ws2Handle, "send"));

	ULONG IntrnlRecv=0, IntrnlSend=0;

	for (ULONG i=NetBase-0x0100; i<NetBase+0x0100; i+=4)
	{
		__try
		{
			ULONG Value = *reinterpret_cast<ULONG*>(i);

			if (Value == WS2Recv)
			{
				IntrnlRecv = i;
				g_Intrnl.Addr[IA_RECV] = i;
			}
			if (Value == WS2Send)
			{
				IntrnlSend = i;
				g_Intrnl.Addr[IA_SEND] = i;
			}
		}
		__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
		{
			continue;
		}

		if (IntrnlRecv&IntrnlSend)
			break;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver3 IntrnlRecv=%08X %08X\n"), IntrnlRecv, WS2Recv);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver3 IntrnlSend=%08X %08X\n"), IntrnlSend, WS2Send);
}

// IA_SCENEBASE, IP_PUSHTEXT

void Resolver_Ph4()
{
	// B9********	// mov	ecx, SCENEBASE
	// 6A00			// push	0
	// 68FFFF9600	// push	96FFFFh
	// 52			// push	edx
	// 6A01			// push	1
	// E8********	// call	PUSHTEXT

	// 6A00			// push	0
	// 68FFFF9600	// push	96FFFFh
	// 51			// push	ecx
	// 6A01			// push	1
	// B9********	// mov	ecx, SCENEBASE
	// E8********	// call	PUSHTEXT

	ULONG Pos = 0;
	UCHAR Code[] = {
		0x68, 0xFF, 0xFF, 0x96, 0x00,	// push	96FFFFh
	};

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return;

	ULONG SceneBase=0, FnPushText=0;

	for (int i=-7; i<16;)
	{
		switch (*reinterpret_cast<UCHAR*>(Pos+i))
		{
			case 0x50:		// push	eax
			case 0x51:		// push	ecx
			case 0x52:		// push	edx
			case 0x53:		// push	ebx
			case 0x54:		// push	esp
			case 0x55:		// push	ebp
			case 0x56:		// push	esi
			case 0x57:		// push	edi
				i += 1;
				break;
			case 0x6A:		// push	imm8
				i += 2;
				break;
			case 0xB9:		// mov	ecx, imm32
				SceneBase = *reinterpret_cast<ULONG*>(Pos+i+1);
				g_Intrnl.Addr[IA_SCENEBASE] = SceneBase;
				i += 5;
				break;
			case 0xE8:		// call	rel32
				// NEWEIP(CALL) = CUREIP + RELATIVE
				FnPushText = Pos + i + 5 + (*reinterpret_cast<ULONG*>(Pos+i+1));
				g_Intrnl.Addr[IP_PUSHTEXT] = FnPushText;
				break;

			default:
				i += 1;
				break;
		}

		if (SceneBase&FnPushText)
			break;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver4 Pos=%08X\n"), Pos);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver4 SceneBase=%08X\n"), SceneBase);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver4 PushText=%08X\n"), FnPushText);
}

// IA_GAMEBASE

void Resolver_Ph5()
{
	// 68********	// push	"login.rsw"
	// 6A00			// push	0
	// B9********	// mov	ecx, GAMEBASE

	// 68********	// push	"login.rsw"
	// 57			// push	edi
	// B9********	// mov	ecx, GAMEBASE

	ULONG Pos = 0;

	if (FindAddStack("login.rsw", &Pos) == false)
		return;

	ULONG GameBase = 0;

	for (int i=0; i<16;)
	{
		switch (*reinterpret_cast<UCHAR*>(Pos+i))
		{
			case 0x50:		// push	eax
			case 0x51:		// push	ecx
			case 0x52:		// push	edx
			case 0x53:		// push	ebx
			case 0x54:		// push	esp
			case 0x55:		// push	ebp
			case 0x56:		// push	esi
			case 0x57:		// push	edi
				i += 1;
				break;
			case 0x6A:		// push	imm8
				i += 2;
				break;
			case 0xB9:		// mov	ecx, imm32
				GameBase = *reinterpret_cast<ULONG*>(Pos+i+1);
				g_Intrnl.Addr[IA_GAMEBASE] = GameBase;
				i += 5;
				break;

			default:
				i += 1;
				break;
		}

		if (GameBase)
			break;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver5 Pos=%08X\n"), Pos);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver5 GameBase=%08X\n"), GameBase);
}

// IP_GETMSGPTR

void Resolver_Ph6()
{
	// 6A00			// push	0
	// 6864FFFF00	// push	0FFFF64h
	// 68********	// push	imm32
	// E8********	// call	GETMSGPTR

	ULONG Pos = 0;
	UCHAR Code[] = {
		0x68, 0x64, 0xFF, 0xFF, 0x00,	// push	0FFFF64h
	};

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return;

	ULONG FnGetMsgPtr = 0;

	if (ResolveEIP(Pos+10, &FnGetMsgPtr))
		g_Intrnl.Addr[IP_GETMSGPTR] = FnGetMsgPtr;

	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver6 Pos=%08X+%08X\n"), Pos, 10);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver6 GetMsgPtr=%08X\n"), FnGetMsgPtr);
}

////////////////////////////////////////////////////////////////////////////////

// IA_CMDTABLE

void Resolver_Ph7()
{
	// FF24BD********	// jmp	[IA_CMDTABLE+edi*4]
	// 8D450E			// lea	eax, [ebp+0Eh]
	// 66C7450ED300		// mov	word ptr [ebp+0Eh], 0D3h

	ULONG Pos = 0;
	UCHAR Code[] = {
		0x66, 0xC7, 0x45, 0x0E, 0xD3, 0x00,	// mov	word ptr [ebp+0Ch+2], 0D3h
	};

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return;

	ULONG CmdTable = 0;

	if (CompareBinary<USHORT>(Pos-10, 0x24FF))				// jmp	r/m32
	{
		CmdTable = *reinterpret_cast<ULONG*>(Pos-7);
		g_Intrnl.Addr[IA_CMDTABLE] = CmdTable;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver7 CmdTable=%08X\n"), CmdTable);
}

////////////////////////////////////////////////////////////////////////////////

// IA_DATABASE, IP_LOADGRF

void Resolver_Ph8()
{
	// 68********	// push	"sdata.grf"
	// B9********	// mov	ecx, IA_DATABASE
	// E8********	// cal	IP_LOADGRF

	ULONG Pos = 0;

	if (FindAddStack("sdata.grf", &Pos) == false)
		return;

	ULONG DataBase = 0;
	ULONG FnLoadGrf = 0;

	if (CompareBinary<UCHAR>(Pos+5, 0xB9))					// mov ecx, imm32
	{
		DataBase = *reinterpret_cast<ULONG*>(Pos+6);
		g_Intrnl.Addr[IA_DATABASE] = DataBase;
	}

	if (ResolveEIP(Pos+10, &FnLoadGrf))
		g_Intrnl.Addr[IP_LOADGRF] = FnLoadGrf;


	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver8 DataBase=%08X\n"), DataBase);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver8 LoadGrf=%08X\n"), FnLoadGrf);
}

////////////////////////////////////////////////////////////////////////////////

// IA_AID, IO_CONDITION1/2

void Resolver_Ph9()
{
	// 8B15********		// mov		edx, [IA_AID]
	// 8B4304			// mov		eax, [ebx+4]
	// 3BC2				// cmp		eax, edx
	// 7522				// jne		rel8
	// 8B45F0			// mov		eax, [ebp-10h]
	// 6A00				// push		0
	// 8B88C4000000		// mov		ecx, [eax+0C4h]
	// 33C0				// xor		eax, eax
	// 8A4308			// mov		al, [ebx+8]
	// 8B493C			// mov		ecx, [ecx+03Ch]
	// 50				// push		eax
	// 0FBF4302			// movsx	eax, word ptr [ebx+2]
	// 8B11				// mov		edx, [ecx]
	// 50				// push		eax
	// 6A6E				// push		6Eh
	// 6A00				// push		0
	// FF5208			// call		[edx+8]

	ULONG Pos = 0;
	UCHAR Code[] = {
		0x6A, 0x6E,			// push	6Eh
		0x6A, 0x00,			// push	0
		0xFF, 0x52, 0x08,	// call	[edx+8]
	};

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return;

	ULONG Aid = 0;
	ULONG Offset1=0, Offset2=0;

	// OFFSET-28h
//	if (CompareBinary<USHORT>(Pos-40, 0x158B))				// mov	edx, [n]
	if (CompareBinary<UCHAR>(Pos-40, 0x8B))				// mov	r32, r/m32
	{
		Aid = *reinterpret_cast<ULONG*>(Pos-38);
		g_Intrnl.Addr[IA_AID] = Aid;
	}
	// OFFSET-16h
//	if (CompareBinary<USHORT>(Pos-22, 0x888B))				// mov	ecx, [eax+n]
	if (CompareBinary<UCHAR>(Pos-22, 0x8B))				// mov	r32, r/m32
	{
		Offset1 = *reinterpret_cast<ULONG*>(Pos-20);
		Offset1 &= 0x000000FF;
		g_Intrnl.Addr[IO_CONDITION1] = Offset1;
	}
	// OFFSET-0Bh
//	if (CompareBinary<USHORT>(Pos-11, 0x498B))				// mov	ecx, [ecx+n]
	if (CompareBinary<UCHAR>(Pos-11, 0x8B))				// mov	r32, r/m32
	{
		Offset2 = *reinterpret_cast<UCHAR*>(Pos-9);
		Offset2 &= 0x000000FF;
		g_Intrnl.Addr[IO_CONDITION2] = Offset2;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver9 Aid=%08X\n"), Aid);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver9 Offset1=%08X\n"), Offset1);
	DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver9 Offset2=%08X\n"), Offset2);
}

////////////////////////////////////////////////////////////////////////////////

// IA_NPBASE

void Resolver_PhA()
{
	// 8138BD04EFFE		// cmp		dword ptr [eax], FEEF04BD
	// 7543				// jne		$+43h
	// 391DCCAD7900		// cmp		[0079ADCC], ebx
	// 753B				// je		$+3Bh

	ULONG Pos = 0;
	UCHAR Code[] = {
		0xBD, 0x04, 0xEF, 0xFE,
		0x75,
	};

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return;

	ULONG NPBase = 0;

	if (CompareBinary<UCHAR>(Pos+6, 0x39))					// cmp	r/m32, r32
	{
		NPBase = *reinterpret_cast<ULONG*>(Pos+8);
		g_Intrnl.Addr[IA_NPBASE] = NPBase;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF ResolverA NPBase=%08X\n"), NPBase);
}

////////////////////////////////////////////////////////////////////////////////

// CMD

ULONG ResolveCMDIndex(PSTR String)
{
	ULONG Pos = 0;

	if (FindStr(String, &Pos) == false)
		return 0;

	// C745F4********	// mov	dword ptr [ebp-0Ch], CMDString
	// C745F8********	// mov	dword ptr [ebp-8], CMDIndex

	// C745F8********	// mov	dword ptr [ebp-8], CMDIndex
	// C745FC********	// mov	dword ptr [ebp-4], CMDString

	UCHAR Code[] = {
		0xC7, 0x45, 0xFC, 0x00, 0x00, 0x00, 0x00,		// mov	dword ptr [ebp-4], 0
	};
	*reinterpret_cast<ULONG*>(Code+3) = Pos;

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return 0;

	ULONG CmdIndex = 0;
	
	if (CompareBinary<UCHAR>(Pos-7, 0xC7))					// mov	r/m32, imm32
		CmdIndex = *reinterpret_cast<ULONG*>(Pos-4);

	return CmdIndex;
}

ULONG ResolveCMD(ULONG CmdTable, ULONG CmdIndex)
{
	ULONG Result = 0;

	__try
	{
		ULONG Label = *reinterpret_cast<ULONG*>(CmdTable+(CmdIndex<<2));

//		if (CompareBinary<USHORT>(Label, 0x0D8B))			// mov	eax, x
		if (CompareBinary<UCHAR>(Label, 0x8B))				// mov	r32, r/m32
			Result = *reinterpret_cast<ULONG*>(Label+2);
//		if (CompareBinary<USHORT>(Label, 0x158B))			// mov	edx, x
//			Result = *reinterpret_cast<ULONG*>(Label+2);

		DbgPrintW(DBGINF, _CRT_WIDE("INF ResolveCMD INDEX=%08X LABEL=%08X, ADDR=%08X\n"), CmdIndex, Label, Result);
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
	{
		Result = 0;
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////

#ifndef PLT_SEARCHRANGE
#define PLT_SEARCHRANGE 0x00030000
#endif

void ResolvePLT(ULONG NetBase)
{
	ULONG Result = 0;

	for (ULONG i=NetBase; i<NetBase+PLT_SEARCHRANGE; i+=4)
	{
		__try
		{
			ULONG Address = *reinterpret_cast<ULONG*>(i);
			PTR_PLT_ENTRY Current = reinterpret_cast<PTR_PLT_ENTRY>(Address);

			if (Current == NULL)
				continue;
			if (Current->Color != 0)
				continue;
			if (Current->Left)
			{
				if (Current->Left->Color ==1)
				{
					Result = Address;
					g_Intrnl.Addr[IA_PLT] = Result;

					DbgPrintW(DBGINF, _CRT_WIDE("INF ResolvePLT Pos=%08X %08X\n"), i, i-4);
					DbgPrintW(DBGINF, _CRT_WIDE("INF ResolvePLT PLT=%08X\n"), Result);
				}
			}
		}
		__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
		{
			continue;
		}

		if (Result)
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////


void Resolver()
{
	Resolver_Ph1();
	Resolver_Ph2();
	Resolver_Ph3();										// Ptrn2ˆË‘¶
	Resolver_Ph4();
	Resolver_Ph5();
	Resolver_Ph6();
	Resolver_Ph7();
	Resolver_Ph8();
	Resolver_Ph9();
	Resolver_PhA();

	if (ULONG CmdTable = g_Intrnl.Addr[IA_CMDTABLE])	// Ptrn7ˆË‘¶
	{
		if (ULONG Addr = ResolveCMD(CmdTable, 0xA6))
			g_Intrnl.Addr[IA_CMDSH] = Addr;
		if (ULONG Addr = ResolveCMD(CmdTable, 0x8C))
			g_Intrnl.Addr[IA_CMDWI] = Addr;

		DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver /sh=%08X\n"), g_Intrnl.Addr[IA_CMDSH]);
		DbgPrintW(DBGINF, _CRT_WIDE("INF Resolver /wi=%08X\n"), g_Intrnl.Addr[IA_CMDWI]);
	}

	if (g_Intrnl.Addr[IA_NETBASE])						// Ptrn2ˆË‘¶
		ResolvePLT(g_Intrnl.Addr[IA_NETBASE]);
}
