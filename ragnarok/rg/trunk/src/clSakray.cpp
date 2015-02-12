#include "intrnl.h"
#include "NTPE.h"

////////////////////////////////////////////////////////////////////////////////

// IA_INITNP

void Resolver_Sak1()
{
	// E8********	// call	INITNP
	// 85C0			// test	eax, eax
	// 75**			// jne	rel8
	// 5F			// pop	edi
	// 5E			// pos	esi
	// B801000000	// mov	eax, 1
	// 5B			// pop	ebx
	// 8B5E			// mov	esp, ebp
	// 5D			// pop	ebp
	// C3			// ret
	// 68********	// push	"resNameTable.txt"

	ULONG Pos = 0;

	if (FindAddStack("resNameTable.txt", &Pos) == false)
		return;

	ULONG InitNP = 0;

	if (CompareBinary<UCHAR>(Pos-21, 0xE8))				// call	rel32
	{
		InitNP = Pos - 21;
		g_Intrnl.Addr[IA_INITNP] = InitNP;
	}
	if (CompareBinary<UCHAR>(Pos-23, 0xE8))				// call	rel32
	{
		InitNP = Pos - 23;
		g_Intrnl.Addr[IA_INITNP] = InitNP;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF ResolverSak1 InitNP=%08X\n"), InitNP);
}

// IA_EFFTBL1, IA_EFFTBL2

void Resolver_Sak2()
{
	// 83C0F6			// add	eax, -Ah
	// 3D********		// cmp	eax, imm32
	// 0F87********		// ja	rel32
	// 33C9				// xor	ecx, ecx
	// 8A88********		// mov	cl, [eax+IA_EFFTBL1]
	// FF248D********	// jmp	[IA_EFFTBL2+ecx*4]
	// A1********		// mov	eax, [IA_MINEFFECT]

	ULONG Pos = 0;
	UCHAR Code[] = {
		0x83, 0xC0, 0xF6,	// add	eax, -Ah
		0x3D,				// cmp	eax, imm32
	};

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return;

	ULONG EffectTable = 0;
	ULONG JumpTable = 0;
	ULONG MinEffect = 0;

//	if (CompareBinary<USHORT>(Pos+16, 0x888A))
	if (CompareBinary<UCHAR>(Pos+16, 0x8A))				// mov	r8, r/m8
	{
		EffectTable = *reinterpret_cast<ULONG*>(Pos+18);
		g_Intrnl.Addr[IA_EFFTBL1] = EffectTable;
	}
//	if (CompareBinary<USHORT>(Pos+22, 0x24FF))			// jmp	r/m32
	if (CompareBinary<UCHAR>(Pos+22, 0xFF))				// jmp	r/m32
	{
		JumpTable = *reinterpret_cast<ULONG*>(Pos+25);
		g_Intrnl.Addr[IA_EFFTBL2] = JumpTable;
	}
	if (CompareBinary<UCHAR>(Pos+29, 0xA1))				// mov	eax, moffs32
	{
		MinEffect = *reinterpret_cast<ULONG*>(Pos+30);
		g_Intrnl.Addr[IA_MINEFFECT] = MinEffect;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF ResolverSak2 EffectTable=%08X\n"), EffectTable);
	DbgPrintW(DBGINF, _CRT_WIDE("INF ResolverSak2 JumpTable=%08X\n"), JumpTable);
	DbgPrintW(DBGINF, _CRT_WIDE("INF ResolverSak2 MinEffect=%08X\n"), MinEffect);
}

// IA_MINEFFECT

void Resolver_Sak3()
{
	// A1********	// mov	eax, [IA_MINEFFECT]
	// 85C0			// test	eax, eax
	// 740A			// jz	imm8
	// 68********	// push	"magnificat_min.str"
	// E9********	// jmp	rel32

	ULONG Pos = 0;

	if (FindAddStack("magnificat_min.str", &Pos) == false)
		return;

	ULONG MinEffect = 0;

	if (CompareBinary<UCHAR>(Pos-9, 0xA1))				// mov	eax, moffs32
	{
		MinEffect = *reinterpret_cast<ULONG*>(Pos-8);
		g_Intrnl.Addr[IA_MINEFFECT] = MinEffect;
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF ResolverSak3 MinEffect=%08X\n"), MinEffect);
}

// IA_CHKNAME

void Resolver_Sak4()
{
	// A1********		// mov	eax, [IA_LANGTYPE]
	// 85C0				// test	eax, eax
	// 0F84B7000000		// je	rel32
	// 83F803			// cmp	eax, 3
	// 0F84AE000000		// je	rel32
	// 83F805			// cmp	eax, 5
	// 0F84A5000000		// je	rel32

	ULONG Pos = 0;
	UCHAR Code[] = {
		0x83, 0xF8, 0x03,
		0x0F, 0x84, 0xAE, 0x00, 0x00, 0x00,
	};

	if (FindBin(Code, sizeof(Code), &Pos) == false)
		return;

	ULONG ChkName = 0;

	if (CompareBinary<USHORT>(Pos+3, 0x840F))				// je	rel32
	{
		ULONG Relative = *reinterpret_cast<ULONG*>(Pos+5);
		Pos = (Pos+9) + Relative;

		// B9********		// mov	ecx, imm32
		// E8********		// call	rel32
		// A1********		// mov	eax, [IA_LANGTYPE]
		// 85C0				// test	eax, eax
		// 0F85********		// jne	rel32

		if (CompareBinary<UCHAR>(Pos+15, 0x85))			// test	r/m32, r32
		{
			ChkName = Pos + 15;
			g_Intrnl.Addr[IA_CHKNAME] = ChkName;
		}
	}

	DbgPrintW(DBGINF, _CRT_WIDE("INF ResolverSak4 ChkName=%08X\n"), ChkName);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Function	: DisableCheckName
// Purpose	:

void DisableCheckName()
{
	if (g_Intrnl.Addr[IA_CHKNAME] == 0)
		return;

	ULONG Pos = g_Intrnl.Addr[IA_CHKNAME];
	UCHAR FixedCode[] = {
		0x85, 0xED,		// test	ebp, ebp
	};

	ForceCopyMemory(reinterpret_cast<PVOID>(Pos), FixedCode, sizeof(FixedCode));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Function	: DisableNP
// Purpose	:

void DisableNP()
{
	if (g_Intrnl.Addr[IA_INITNP] == 0)
		return;

	// ORIGINAL
	// E8********	// call	INITNP
	// 85C0			// test	eax, eax
	// 75**			// jne	rel8

	ULONG Pos = g_Intrnl.Addr[IA_INITNP];
	UCHAR FixedCode[] = {
		0x8B, 0xC5,		// mov	eax, ebp
		0xEB, 0x01,		// jmp	rel8
		0x90,			// nop
	};

	ForceCopyMemory(reinterpret_cast<PVOID>(Pos), FixedCode, sizeof(FixedCode));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void SakrayResolver()
{
//	Resolver_Sak1();
	Resolver_Sak2();
//	Resolver_Sak3();

//	DisableCheckName();
//	DisableNP();
}
