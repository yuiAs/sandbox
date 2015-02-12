#include "intrnl.h"
#include <stdio.h>

#ifdef DBG
#pragma warning(disable: 4312)
#endif

////////////////////////////////////////////////////////////////////////////////

// Function	: SetConditionIcon
// Purpose	:

void SetConditionIcon(USHORT TypeID, UCHAR Flag)
{
	ULONG GameBase = g_Intrnl.Addr[IA_GAMEBASE];
	ULONG Offset1 = g_Intrnl.Addr[IO_CONDITION1];
	ULONG Offset2 = g_Intrnl.Addr[IO_CONDITION2];

	__try
	{
		__asm
		{
			mov		esi, GameBase
			mov		edi, [esi+4]
			push	0
			mov		edx, Offset1
			mov		ecx, [edi+edx]
			xor		eax, eax
			mov		al, byte ptr [Flag]
			mov		esi, ecx
			mov		edx, Offset2
			mov		ecx, [esi+edx]
			push	eax
			xor		eax, eax
			mov		ax, word ptr [TypeID]
			mov		edx, [ecx]
			push	eax
			push	6Eh
			push	0
			call	dword ptr [edx+8]
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrintW(DBGERR, _CRT_WIDE("ERR Raise exception at SetConditionIcon Type=%04X Flag=%d\n"), TypeID, Flag);
	}
}

////////////////////////////////////////////////////////////////////////////////

// Function	: LoadGrf
// Purpose	:

void LoadGrf(PSTR FileName)
{
	ULONG Function = g_Intrnl.Addr[IP_LOADGRF];
	ULONG DataBase = g_Intrnl.Addr[IA_DATABASE];

	__asm
	{
		push	FileName
		mov		ecx, DataBase
		call	dword ptr [Function]
	}
}

////////////////////////////////////////////////////////////////////////////////

// Function	: GetNetBase
// Purpose	:
// Return	:

ULONG GetNetBase()
{
	ULONG Function = g_Intrnl.Addr[IP_GETNETBASE];

	__asm
	{
		call	dword ptr [Function]
	}
}

////////////////////////////////////////////////////////////////////////////////

#ifndef PUSHTEXT_MAX
#define PUSHTEXT_MAX 80
#endif

// Function	: _PushText
// Purpose	:
// Arguments:
// Return	:

bool _PushText(ULONG Color, PSTR String)
{
	if (g_Intrnl.DisableEXMsg)
		return false;

	ULONG Function = g_Intrnl.Addr[IP_PUSHTEXT];
	ULONG SceneBase = g_Intrnl.Addr[IA_SCENEBASE];

	__asm
	{
		push	0
		push	0
		push	Color
		push	String
		push	1
		mov		ecx, SceneBase
		call	dword ptr [Function]
	}

	return true;
}

// Function	: CWPushText
// Purpose	:
// Arguments:
// Return	:

bool CWPushText(ULONG Color, PSTR String)
{
	if (strlen(String) > PUSHTEXT_MAX)
		return false;
	else
		return _PushText(Color, String);
}

// Function	: CWPushFormattedText
// Purpose	:
// Arguments:
// Return	:

bool CWPushFormattedText(ULONG Color, PSTR Format, ...)
{
	char Buffer[PUSHTEXT_MAX];
	RtlZeroMemory(Buffer, PUSHTEXT_MAX);	// 80byte–Ú‚ðŠmŽÀ\0‚É‚·‚é‚½‚ß

	va_list va;
	va_start(va, Format);
	int Result = _vsnprintf(Buffer, PUSHTEXT_MAX-1, Format, va);
	va_end(va);

	if (Result == -1)
		return false;

	return _PushText(Color, Buffer);
}

////////////////////////////////////////////////////////////////////////////////

// Function	: GetMsgStringTablePtr
// Purpose	:
// Arguments:

void GetMsgStringTablePtr(ULONG StringID, PVOID* PtrBuffer)
{
	ULONG Function = g_Intrnl.Addr[IP_GETMSGPTR];

	__asm
	{
		push	StringID
		call	dword ptr [Function]
		add		esp, 4
		mov		edx, dword ptr [PtrBuffer]
		mov		[edx], eax
	}
}

////////////////////////////////////////////////////////////////////////////////

// Function	: _GetParsingFunctionAddress
// Purpose	:
// Arguments:
// Return	:

ULONG _GetParsingFunctionAddress(USHORT Op)
{
	ULONG JmpTbl = g_Intrnl.Addr[IA_ZONETABLE];

	__asm
	{
		mov		esi, JmpTbl
		xor		eax, eax
		mov		ax, word ptr [Op]
		add		eax, 0FFFFFF8Dh
		mov		edi, [esi+eax*4]
		xor		ebx, ebx
		mov		bl, byte ptr [edi+9]
		cmp		ebx, 0E8h
		xor		eax, eax
		jne		UNEXPECTED
		mov		edx, [edi+0Ah]
		lea		eax, [edi+edx+0Eh]
UNEXPECTED:
	}
}

// Function	: ExecuteParser
// Purpose	:
// Arguments:
// Return	:

bool ExecuteParser(USHORT Op, PVOID Buffer)
{
	ULONG Function = _GetParsingFunctionAddress(Op);
	if (Function == 0)
		return false;

	bool Result = false;
	ULONG GameBase = g_Intrnl.Addr[IA_GAMEBASE];

	__try
	{
		__asm
		{
			mov		eax, GameBase
			mov		ecx, [eax+4]
			push	Buffer
			call	[Function]
		}

		Result = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrintW(DBGERR, _CRT_WIDE("ERR Raise exception at ExecuteParser, Op=%04X Function=%08X\n"), Op, Function);
		Result = false;
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////

// Function	: SetClCmdValue
// Purpose	:
// Arguments:
// Return	:

bool SetClCmdValue(ULONG Address, ULONG Value)
{
	if (Address == 0)
		return false;

	bool Result = false;

	__try
	{
		InterlockedExchange(reinterpret_cast<LONG*>(Address), Value);
		Result = true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrintW(DBGERR, _CRT_WIDE("ERR Raise exception at SetCmdValue, Address=%08X Value=%d\n"), Address, Value);
		Result = false;
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////

// Function	: StopCRCCheck
// Purpose	:
// Arguments:
// Return	:

void StopCRCCheck()
{
	ULONG NPBase = g_Intrnl.Addr[IA_NPBASE];
	HANDLE NPCRCEvent = NULL;

	_asm
	{
		mov		eax, NPBase
		mov		edx, [eax]
		mov		eax, [edx+18h]
		push	eax
		call	ResetEvent
	}
}


