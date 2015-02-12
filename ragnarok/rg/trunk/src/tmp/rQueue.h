#ifndef RQUEUE_H
#define RQUEUE_H

#include <windows.h>
#include "DLinkList.h"

////////////////////////////////////////////////////////////////////////////////


class ROQueue
{
private:

	ROQueue(const ROQueue& _class);
	ROQueue& operator=(const ROQueue& _class);

private:

	bool m_Terminate;

private:

	typedef struct _QUEUE
	{
		void* Function;
		void* Args;
		ULONG Delay;
		LIST_ENTRY ListEntry;
	} QUEUE, *PTR_QUEUE;

	LIST_ENTRY m_QueLinks;

private:

	void Init();
	void Release();

public:

	bool Add(void* PtrFunction, void* PtrArgs, ULONG Delayed);

	void Suspend();
	void Resume();

private:

	void CheckQueue();

public:

	ULONG Worker();

};


#endif	// #ifndef RQUEUE_H
