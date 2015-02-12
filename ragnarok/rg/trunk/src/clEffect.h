#ifndef CLEFFECT_H
#define CLEFFECT_H

#include "win32.h"

////////////////////////////////////////////////////////////////////////////////


class EffectController
{
private:
	
	EffectController(const EffectController& _class);
	EffectController& operator=(const EffectController& _class);

private:

	ULONG m_TypeTable;
	ULONG m_JumpTable;
	ULONG m_MinEffect;

private:

	typedef struct _EFFECTOR_ITEM
	{
		USHORT Id;
		UCHAR* Code;
		SLIST_ENTRY ListEntry;
	} EFFECTOR_ITEM, *PTR_EFFECTOR_ITEM;

	SLIST_HEADER m_EffectorItems;

public:

	EffectController();
	virtual ~EffectController();

public:

	void Init(ULONG TypeTable, ULONG JumpTable, ULONG MinEffect);
	bool Disabled(USHORT EffectID);

private:

	void Release();

	ULONG GetLabel(USHORT EffectID);
	ULONG SetLabel(USHORT EffectID, ULONG NewLabel);
};


/*
#include <windows.h>
//#include "DLinkList.h"

////////////////////////////////////////////////////////////////////////////////


class EffectController
{
private:
	
	EffectController(const EffectController& _class);
	EffectController& operator=(const EffectController& _class);

private:

	PVOID m_VM;
	ULONG m_VMPos;

	ULONG m_EffectTable;
	ULONG m_JumpTable;
	ULONG m_MinEffect;

public:

	EffectController();
	virtual ~EffectController();

public:

	bool Init(ULONG EffectTable, ULONG JumpTable, ULONG MinEffect);

	bool DisableEffect(USHORT EffectID);
	bool EnableEffect(USHORT EffectID);

private:

	void Release();

	void ChangeVMProtect(ULONG Protect);
	ULONG GetLabelAddress(USHORT EffectID);
	ULONG SetLabelAddress(USHORT EffectID, ULONG Address);
	
};
*/


#endif	// #ifndef CLEFFECT_H
