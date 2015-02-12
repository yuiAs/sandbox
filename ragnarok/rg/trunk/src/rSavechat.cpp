#include "rSavechat.h"
#include <stdio.h>
#include "client.h"
#include "NTTime.hpp"
#include "debug.h"

#ifndef LOGCOL_MAX
#define LOGCOL_MAX 1024
#endif

////////////////////////////////////////////////////////////////////////////////


ROSavechat::ROSavechat() : m_File(NULL), m_Heap(NULL)
{
	Init();
}

ROSavechat::~ROSavechat()
{
	Release();
}

////////////////////////////////////////////////////////////////////////////////

// Function	: Init
// Purpose	:
// Arguments:
// Return	:

void ROSavechat::Init()
{
	InitializeCriticalSection(&m_Section);
	InitializeListHead(&m_SaveLogItems);

	// Heap

	if (m_Heap == NULL)
		m_Heap = HeapCreate(0, 2048<<2, 0);
	if (m_Heap == NULL)
	{
		DbgPrintW(DBGERR, _CRT_WIDE("ERR Failure HeapCreate at ROSavechat::Init, and use ProcessHeap.\n"));
		m_Heap = GetProcessHeap();
	}
	else
	{
		ULONG LFHValue = 2;
		HeapSetInformation(m_Heap, HeapCompatibilityInformation, &LFHValue, sizeof(ULONG));
	}

	// File

	if (m_File == NULL)
		m_File = new FileOut;

	CreateDirectoryW(_CRT_WIDE("Chat"), NULL);
	SwitchFile(NULL);
}

// Function	: Release
// Purpose	:
// Arguments:
// Return	:

void ROSavechat::Release()
{
	Runnable();

	if (m_Heap != NULL)
	{
		if (m_Heap != GetProcessHeap())
			HeapDestroy(m_Heap);
		// ProcessHeapで作ってる時は終了と同時に消えてもらう
	}

	if (m_File)
		delete m_File;

	DeleteCriticalSection(&m_Section);
}

// Function	: Runnable
// Purpose	:
// Arguments:
// Return	:
// Note		: CriticalSectionのKernelMode遷移コストがあれなので修正したい。。

ULONG ROSavechat::Runnable()
{
	if (TryEnterCriticalSection(&m_Section))
	{
		while (IsListEmpty(&m_SaveLogItems)==FALSE)
			Pop();

		LeaveCriticalSection(&m_Section);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

// Function	: SwitchFile
// Purpose	:
// Arguments:
// Return	:

void ROSavechat::SwitchFile(FILETIME* CurTime)
{
	if (m_File == NULL)
		return;

	if (m_File->IsOpend() && (CurTime!=NULL))
	{
		if (CompareFileTime(&m_ChgTime, CurTime) > 0)
			return;
		else
			m_File->Close();	// 閉じなくてもOpenで勝手に閉じる
	}

	SYSTEMTIME LcTime;
	GetLocalTime(&LcTime);

	wchar_t FileName[MAX_PATH];
	_snwprintf(FileName, MAX_PATH-1, _CRT_WIDE("%s/%04d-%02d-%02d.txt"), _CRT_WIDE("Chat"), LcTime.wYear, LcTime.wMonth, LcTime.wDay);

	DbgPrintW(DBGINF, _CRT_WIDE("INF CurrentLogFile=%s\n"), FileName);

	bool Result = m_File->Open(FileName);
	if (Result)
	{
		LcTime.wHour = 0;
		LcTime.wMinute = 0;
		LcTime.wSecond = 0;
		LcTime.wMilliseconds = 0;

		SystemTimeToFileTimeAdd(&LcTime, ConvertSecondToNanoSecond(24*60*60), &m_ChgTime);
	}
}

////////////////////////////////////////////////////////////////////////////////

// Function	: Push
// Purpose	:
// Arguments:
// Return	:

void ROSavechat::Push(const PUCHAR Buffer, int Length)
{
	EnterCriticalSection(&m_Section);

	PTR_SAVELOG_ITEM Queue = reinterpret_cast<PTR_SAVELOG_ITEM>(HeapAlloc(m_Heap, HEAP_ZERO_MEMORY, sizeof(SAVELOG_ITEM)));
	if (Queue)
	{
		Queue->Data = reinterpret_cast<PUCHAR>(HeapAlloc(m_Heap, HEAP_ZERO_MEMORY, Length));
		RtlCopyMemory(Queue->Data, Buffer, Length);
		GetLocalTimeAsFileTime(&Queue->Stamp);

		InsertTailList(&m_SaveLogItems, &Queue->ListEntry);
	}

	LeaveCriticalSection(&m_Section);
}

// Function	: Pop
// Purpose	:
// Arguments:
// Return	:

void ROSavechat::Pop()
{
	PLIST_ENTRY Entry = RemoveHeadList(&m_SaveLogItems);
	PTR_SAVELOG_ITEM Queue = reinterpret_cast<PTR_SAVELOG_ITEM>(CONTAINING_RECORD(Entry, SAVELOG_ITEM, ListEntry));

	SwitchFile(&Queue->Stamp);
	Formatted(Queue);
	ReleaseStack(Queue);
}

// Function	: ReleaseStack
// Purpose	:
// Arguments:
// Return	:

void ROSavechat::ReleaseStack(PTR_SAVELOG_ITEM Queue)
{
	HeapFree(m_Heap, 0, Queue->Data);
	HeapFree(m_Heap, 0, Queue);
}

////////////////////////////////////////////////////////////////////////////////

// Function	: Formatted
// Purpose	:
// Arguments:
// Return	:

void ROSavechat::Formatted(PTR_SAVELOG_ITEM Queue)
{
	PUCHAR Raw = Queue->Data;

	SYSTEMTIME LogTime;
	FileTimeToSystemTime(&Queue->Stamp, &LogTime);

	int Result = -1;

	char Buffer[LOGCOL_MAX];
	RtlZeroMemory(Buffer, sizeof(Buffer));

	switch (*reinterpret_cast<USHORT*>(Raw))
	{
		case PACKET_ZC_CHAT:
			Result = _snprintf(Buffer, LOGCOL_MAX-1, "NOR %02d:%02d:%02d %s\r\n", LogTime.wHour, LogTime.wMinute, LogTime.wSecond, Raw+8);
			break;

		case PACKET_ZC_CHAT_THIS:
			Result = _snprintf(Buffer, LOGCOL_MAX-1, "NOR %02d:%02d:%02d %s\r\n", LogTime.wHour, LogTime.wMinute, LogTime.wSecond, Raw+4);
			break;

		case PACKET_CZ_WHISPER:
			break;

		case PACKET_ZC_WHISPER:
			{
				PSTR Name = reinterpret_cast<PSTR>(Raw+4);
				PSTR Body = reinterpret_cast<PSTR>(Raw+28);

				if (*reinterpret_cast<UCHAR*>(Raw+2) > 32)
				{
					if (*reinterpret_cast<UCHAR*>(Raw+31) < 0x10)
						Body = reinterpret_cast<PSTR>(Raw+32);
				}

				Result = _snprintf(Buffer, LOGCOL_MAX-1, "WIS %02d:%02d:%02d FROM %s : %s\r\n", LogTime.wHour, LogTime.wMinute, LogTime.wSecond, Name, Body);
			}
			break;

		case PACKET_ZC_WHISPER_ACK:
			{
				// line0149:  接続していないか、存在しないキャラクターの名前です。#
				// line0151:  全てのキャラクターに対して受信拒否状態です。#

				PSTR WisMsg = NULL;

				switch (*reinterpret_cast<UCHAR*>(Raw+2))
				{
					case 0x01:
						GetMsgStringTablePtr(MSG_WHIS_FAILED, reinterpret_cast<PVOID*>(&WisMsg));
						break;
					case 0x02:
						GetMsgStringTablePtr(MSG_WHIS_REFUSE, reinterpret_cast<PVOID*>(&WisMsg));
						break;
					default:
						break;
				}

				if (WisMsg)
					Result = _snprintf(Buffer, LOGCOL_MAX-1, "SYS %02d:%02d:%02d %\r\n", LogTime.wHour, LogTime.wMinute, LogTime.wSecond, WisMsg);
			}
			break;

		case PACKET_ZC_BROADCAST:
			Result = _snprintf(Buffer, LOGCOL_MAX-1, "GBC %02d:%02d:%02d %s\r\n", LogTime.wHour, LogTime.wMinute, LogTime.wSecond, Raw+4);
			break;

		case PACKET_ZC_PARTYCHAT:
			Result = _snprintf(Buffer, LOGCOL_MAX-1, "PRT %02d:%02d:%02d %s\r\n", LogTime.wHour, LogTime.wMinute, LogTime.wSecond, Raw+8);
			break;

		case PACKET_ZC_MVP:
			Result = _snprintf(Buffer, LOGCOL_MAX-1, "MVP %02d:%02d:%02d %08X\r\n", LogTime.wHour, LogTime.wMinute, LogTime.wSecond, *reinterpret_cast<ULONG*>(Raw+2));
			break;

		case PACKET_ZC_GUILDCHAT:
			Result = _snprintf(Buffer, LOGCOL_MAX-1, "GLD %02d:%02d:%02d %s\r\n", LogTime.wHour, LogTime.wMinute, LogTime.wSecond, Raw+4);
			break;

		case PACKET_ZC_BROADCAST2:
			Result = _snprintf(Buffer, LOGCOL_MAX-1, "LBC %02d:%02d:%02d %s\r\n", LogTime.wHour, LogTime.wMinute, LogTime.wSecond, Raw+16);
			break;
	}

	if (Result > 0)
		m_File->Write(Buffer, Result);
}
