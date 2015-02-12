#include "clEffect.h"
#include "NTPE.h"
#include "client.h"
#include "debug.h"

////////////////////////////////////////////////////////////////////////////////


EffectController::EffectController() : 
	m_TypeTable(0), m_JumpTable(0), m_MinEffect(0)
{
}

EffectController::~EffectController()
{
	Release();
}

////////////////////////////////////////////////////////////////////////////////


void EffectController::Init(ULONG TypeTable, ULONG JumpTable, ULONG MinEffect)
{
	RtlInitializeSListHead(&m_EffectorItems);

	m_TypeTable = TypeTable;
	m_JumpTable = JumpTable;
	m_MinEffect = MinEffect;

	DbgPrintW(DBGINF, _CRT_WIDE("INF EffectController::Init %08X %08X %08X\n"), m_TypeTable, m_JumpTable, m_MinEffect);
}

void EffectController::Release()
{
	while (PSLIST_ENTRY Entry = RtlInterlockedPopEntrySList(&m_EffectorItems))
	{
		PTR_EFFECTOR_ITEM Item = reinterpret_cast<PTR_EFFECTOR_ITEM>(CONTAINING_RECORD(Entry, EFFECTOR_ITEM, ListEntry));
		VirtualFreeEx(GetCurrentProcess(), Item, 0, MEM_RELEASE);
	}

	RtlInterlockedFlushSList(&m_EffectorItems);
}

////////////////////////////////////////////////////////////////////////////////


bool EffectController::Disabled(USHORT EffectID)
{
	ULONG NoProc = GetLabel(EC_NONE);
	ULONG Target = GetLabel(EffectID);

	if ((NoProc&Target) == 0)
		return false;

	UCHAR Code[] = {
		0xA1, 0x00, 0x00, 0x00, 0x00,	// mov	eax, [IA_MINEFFECT]
		0x85, 0xC0,						// test	eax, eax
		0x74, 0x07,						// je	$+7
		0xB8, 0x00, 0x00, 0x00, 0x00,	// mov	eax, NOPROC
		0xFF, 0xE0,						// jmp	eax
		0xB8, 0x00, 0x00, 0x00, 0x00,	// mov	eax, TARGET
		0xFF, 0xE0,						// jmp	eax
		0x90,
	};

	*reinterpret_cast<ULONG*>(Code+1) = m_MinEffect;
	RtlCopyMemory(Code+10, &NoProc, sizeof(ULONG));
	RtlCopyMemory(Code+17, &Target, sizeof(ULONG));

	// Allocate

	ULONG Size = sizeof(EFFECTOR_ITEM) + sizeof(Code);
	UCHAR* Buffer = reinterpret_cast<UCHAR*>(VirtualAllocEx(GetCurrentProcess(), NULL, Size, MEM_COMMIT, PAGE_EXECUTE_WRITECOPY));
	if (Buffer == NULL)
	{
		DbgPrintW(DBGINF, _CRT_WIDE("ERR VirtualAllocEx at EffectController::Disabled ERROR=%08X\n"), GetLastError());
		return false;
	}

	PTR_EFFECTOR_ITEM Item = reinterpret_cast<PTR_EFFECTOR_ITEM>(Buffer);
	Item->Id = EffectID;
	Item->Code = Buffer + sizeof(EFFECTOR_ITEM);

	RtlFillMemory(Item->Code, sizeof(Code), 0x90);
	RtlCopyMemory(Item->Code, Code, sizeof(Code));

	SetLabel(EffectID, reinterpret_cast<ULONG>(Item->Code));
	RtlInterlockedPushEntrySList(&m_EffectorItems, &Item->ListEntry);

	DbgPrintW(DBGINF, _CRT_WIDE("INF EffectController::Disabled ID=%04X LABEL=%08X ALLOC=%08X NEW=%08X"), EffectID, Target, Buffer, Item->Code);
	return true;
}

////////////////////////////////////////////////////////////////////////////////


ULONG EffectController::SetLabel(USHORT EffectID, ULONG NewLabel)
{
	ULONG Target = GetLabel(EffectID);
	if (Target == 0)
		return 0;

	ULONG PrevLabel = 0;
	MEMORY_BASIC_INFORMATION MemInfo;

	UnlockMemoryProtect(reinterpret_cast<PVOID>(Target), &MemInfo);
	PrevLabel = reinterpret_cast<ULONG>(InterlockedExchangePointer(&Target, NewLabel));
	RevertMemoryProtect(&MemInfo);
	FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<PVOID>(Target), sizeof(ULONG));

	return PrevLabel;
}

ULONG EffectController::GetLabel(USHORT EffectID)
{
	ULONG Label = 0;

	__try
	{
		UCHAR Offset = *reinterpret_cast<UCHAR*>((EffectID-0x0A)+m_TypeTable);
		Label = m_JumpTable + (static_cast<ULONG>(Offset)<<2);

	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
	{
		DbgPrintW(DBGERR, _CRT_WIDE("ERR Raise ACCESS_VIOLATION at EffectController::GetLabel, EffectID=%04X\n"), EffectID);
		Label = 0;
	}

	return Label;
}



/*
#ifndef EFFECTVM_MAX
#define EFFECTVM_MAX 1024<<4
#endif

#ifndef EFFECTID_BASE
#define EFFECTID_BASE 0x0A
#endif

////////////////////////////////////////////////////////////////////////////////


EffectController::EffectController() :
	m_VM(NULL), m_VMPos(0),
	m_EffectTable(0), m_JumpTable(0), m_MinEffect(0)
{
}

EffectController::~EffectController()
{
	Release();
}

////////////////////////////////////////////////////////////////////////////////


bool EffectController::Init(ULONG EffectTable, ULONG JumpTable, ULONG MinEffect)
{
	m_EffectTable = EffectTable;
	m_JumpTable = JumpTable;
	m_MinEffect = MinEffect;

	if (m_VM == NULL)
	{
		m_VM = VirtualAllocEx(GetCurrentProcess(), NULL, EFFECTVM_MAX, MEM_COMMIT, PAGE_EXECUTE_WRITECOPY);
		if (m_VM == NULL)
			return false;
	}

	m_VMPos = 0;
	RtlFillMemory(m_VM, EFFECTVM_MAX, 0x90);

	return true;
}

void EffectController::Release()
{
	if (m_VM)
	{
		VirtualFreeEx(GetCurrentProcess(), m_VM, 0, MEM_RELEASE);

		m_VM = NULL;
		m_VMPos = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////


void EffectController::ChangeVMProtect(ULONG Protect)
{
	ULONG Dummy = 0;
	VirtualProtectEx(GetCurrentProcess(), m_VM, EFFECTVM_MAX, Protect, &Dummy);
}

////////////////////////////////////////////////////////////////////////////////


ULONG EffectController::GetLabelAddress(USHORT EffectID)
{
	ULONG Address = 0;

	__try
	{
		UCHAR Offset = *reinterpret_cast<UCHAR*>((EffectID-0x0A)+m_EffectTable);
		Address = m_JumpTable + (Offset<<2);
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
	{
		DbgPrintW(DBGERR, _CRT_WIDE("ERR Raise ACCESS_VIOLATION at GetLabelAddress, EffectID=%04X\n"), EffectID);
		Address = 0;
	}

	return Address;
}

ULONG EffectController::SetLabelAddress(USHORT EffectID, ULONG Address)
{
	ULONG Target = GetLabelAddress(EffectID);
	if (Target == 0)
		return 0;

	ULONG Prev = 0;
	MEMORY_BASIC_INFORMATION MemInfo;

	UnlockMemoryProtect(reinterpret_cast<PVOID>(Target), &MemInfo);
	Prev = reinterpret_cast<ULONG>(InterlockedExchangePointer(&Target, Address));
	RevertMemoryProtect(&MemInfo);

	FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<PVOID>(Target), sizeof(ULONG));
	
	return Prev;
}

////////////////////////////////////////////////////////////////////////////////


bool EffectController::DisableEffect(USHORT EffectID)
{
	ULONG NoProc = GetLabelAddress(EC_NONE);
	ULONG Target = GetLabelAddress(EffectID);

	if ((NoProc&Target) == 0)
		return false;

	UCHAR Code[] = {
		0xA1, 0x00, 0x00, 0x00, 0x00,	// mov	eax, [IA_MINEFFECT]
		0x85, 0xC0,						// test	eax, eax
		0x74, 0x07,						// je	$+7
		0xB8, 0x00, 0x00, 0x00, 0x00,	// mov	eax, NOPROC
		0xFF, 0xE0,						// jmp	eax
		0xB8, 0x00, 0x00, 0x00, 0x00,	// mov	eax, TARGET
		0xFF, 0xE0,						// jmp	eax
	};
	*reinterpret_cast<ULONG*>(Code+1) = m_MinEffect;
	*reinterpret_cast<ULONG*>(Code+10) = *reinterpret_cast<ULONG*>(NoProc);
	*reinterpret_cast<ULONG*>(Code+17) = *reinterpret_cast<ULONG*>(Target);

	if ((m_VMPos+sizeof(Code)) > EFFECTVM_MAX)
	{
		DbgPrintW(DBGERR, _CRT_WIDE("ERR Overflow VM at EffectController::DisableEffect\n"));
		return false;
	}
	else
	{
		ULONG Original = *reinterpret_cast<ULONG*>(Code+17);
		ULONG VMAddress = reinterpret_cast<ULONG>(m_VM) + m_VMPos;
		
		RtlCopyMemory(reinterpret_cast<PVOID>(VMAddress), Code, sizeof(Code));
		SetLabelAddress(Target, VMAddress);

		m_VMPos += sizeof(Code);

		DbgPrintW(DBGINF, _CRT_WIDE("INF DisableEffect ID=%04X LABEL=%08X OLD=%08X NEW=%08X\n"), EffectID, Target, Original, VMAddress);
		return true;
	}
}

bool EffectController::EnableEffect(USHORT EffectID)
{
	ULONG Target = GetLabelAddress(EffectID);
	if (Target == 0)
		return false;

	ULONG Original = 0;
	ULONG Prev = SetLabelAddress(Target, Original);

	return (Prev!=0);
}
*/
