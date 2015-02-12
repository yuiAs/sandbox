#include "rQueue.h"

////////////////////////////////////////////////////////////////////////////////


ROQueue::ROQueue() : m_Terminate(false)
{
	Init();
}

ROQueue::~ROQueue()
{
	Release();
}

////////////////////////////////////////////////////////////////////////////////


void ROQueue::Init()
{
	InitializeListHead(&m_QueLinks);
}

void ROQueue::Release()
{
	m_Terminate = true;
}

////////////////////////////////////////////////////////////////////////////////


void ROQueue::Add(void* PtrFunction, void* PtrArgs, ULONG Delayed)
{
	PTR_QUEUE Queue = new QUEUE;
//	::RtlZeroMemory(q, sizeof(QUEUE));
	Queue->Function = PtrFunction;
	Queue->Args = PtrArgs;
	Queue->Delayed = Delayed;

	InsertTailList(&m_QueLinks, &Queue->ListEntry);
}

////////////////////////////////////////////////////////////////////////////////


ULONG ROQueue::Worker()
{
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);
	::SuspendThread(::GetCurrentThread());

	while (m_Terminate==false)
	{
		CheckQueue();

		::SwitchToThread();
		::Sleep(800);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


void ROQueue::CheckQueue()
{
	if (IsListEmpty(&m_QueLinks))
		return;

	ULONG Tick = ::GetTickCount();

	PLIST_ENTRY Entry = m_QueLinks.Flink;
	PTR_QUEUE Queue = reinterpret_cast<PTR_QUEUE>(CONTAINING_RECORD(Entry, QUEUE, ListEntry));

	typedef void (

}

