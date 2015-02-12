#include "rParser.h"
#include "client.h"
#include "debug.h"

////////////////////////////////////////////////////////////////////////////////


void ROParser::InitDelayedQueue()
{
	InitializeListHead(&m_QueLinks);

	if (m_Heap == NULL)
		m_Heap = ::HeapCreate(0, 2048<<4, 0);
	if (m_Heap == NULL)
	{
		DbgPrintW(DBGERR, _CRT_WIDE("ERR Failure HeapCreate at ROParser::InitDelayedQueue, and use ProcessHeap.\n"));
		m_Heap = ::GetProcessHeap();
	}
	else
	{
		ULONG LFHValue = 2;
		::HeapSetInformation(m_Heap, HeapCompatibilityInformation, &LFHValue, sizeof(ULONG));
	}
}

void ROParser::ReleaseDelayedQueue(bool Terminate)
{
	while (IsListEmpty(&m_QueLinks)==FALSE)
	{
		PLIST_ENTRY Entry = RemoveHeadList(&m_QueLinks);
		PTR_DELAYEDQUEUE Queue = reinterpret_cast<PTR_DELAYEDQUEUE>(CONTAINING_RECORD(Entry, DELAYEDQUEUE, ListEntry));
		::HeapFree(m_Heap, 0, Queue->Data);
		::HeapFree(m_Heap, 0, Queue);
	}

	if (Terminate && (m_Heap!=NULL))
	{
		if (m_Heap != ::GetProcessHeap())
			::HeapDestroy(m_Heap);
	}
}

////////////////////////////////////////////////////////////////////////////////


void ROParser::AddDelayedQueue(PUCHAR Buffer, int Length, ULONG Delayed)
{
	PTR_DELAYEDQUEUE Queue = reinterpret_cast<PTR_DELAYEDQUEUE>(::HeapAlloc(m_Heap, HEAP_ZERO_MEMORY, sizeof(DELAYEDQUEUE)));
	if (Queue)
	{
/*
		Queue->Data = reinterpret_cast<PUCHAR>(::HeapAlloc(m_Heap, HEAP_ZERO_MEMORY, Length));
		::RtlCopyMemory(Queue->Data, Buffer, Length);
		Queue->Tick = Delayed;

		InsertTailList(&m_QueLinks, &Queue->ListEntry);
*/
		DbgPrintW(DBGINF, _CRT_WIDE("CHK %08X %d %d\n"), Buffer, Length, Delayed);
		::HeapFree(m_Heap, 0, Queue);
	}
}

////////////////////////////////////////////////////////////////////////////////


void ROParser::CheckDelayedQueue(ULONG Tick)
{
	if (IsListEmpty(&m_QueLinks))
		return;

	for (PLIST_ENTRY Entry=m_QueLinks.Flink; Entry!=&m_QueLinks; Entry=Entry->Flink)
	{
		PTR_DELAYEDQUEUE Queue = reinterpret_cast<PTR_DELAYEDQUEUE>(CONTAINING_RECORD(Entry, DELAYEDQUEUE, ListEntry));
		if (Queue->Tick < Tick)
		{
			RemoveEntryList(Entry);

			ExecuteParser(*reinterpret_cast<USHORT*>(Queue->Data), Queue->Data);
			::HeapFree(m_Heap, 0, Queue->Data);
			::HeapFree(m_Heap, 0, Queue);
		}
	}
}
