#ifndef RSAVECHAT_H
#define RSAVECHAT_H

#include "win32.h"
#include "fileOut.hpp"

////////////////////////////////////////////////////////////////////////////////


class ROSavechat
{
private:

	ROSavechat(const ROSavechat& _class);
	ROSavechat& operator=(const ROSavechat& _class);

private:

	FileOut* m_File;
	FILETIME m_ChgTime;

	CRITICAL_SECTION m_Section;
	HANDLE m_Heap;

private:

	typedef struct _SAVELOG_ITEM
	{
		PUCHAR Data;
		FILETIME Stamp;
		LIST_ENTRY ListEntry;
	} SAVELOG_ITEM, *PTR_SAVELOG_ITEM;

	LIST_ENTRY m_SaveLogItems;

public:

	ROSavechat();
	virtual ~ROSavechat();

private:

	void Init();
	void Release();

	void Pop();
	void Formatted(PTR_SAVELOG_ITEM Stack);
	void ReleaseStack(PTR_SAVELOG_ITEM Stack);

	void SwitchFile(FILETIME* CurTime);

public:

	void Push(const PUCHAR Buffer, int Length);
	ULONG Runnable();

};


#endif	// #ifndef RSAVECHAT_H
