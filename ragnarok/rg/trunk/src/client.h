#ifndef CLIENT_H
#define CLIENT_H

#include <windows.h>
#include "clEffect.h"
#include "aegis.h"
#include "clientEnum.h"


// PacketLengthTable

#pragma pack(push,1)
typedef struct _PLT_ENTRY
{
	struct _PLT_ENTRY* Left;
	struct _PLT_ENTRY* Parent;
	struct _PLT_ENTRY* Right;
	ULONG Op;
	ULONG Length;
	ULONG Color;
} PLT_ENTRY, *PTR_PLT_ENTRY;
#pragma pack(pop)	// #pragma pack(push,1)

// clResolver.cpp
// clSakray.cpp

void Resolver();
void SakrayResolver();

// clApi.cpp

bool SetClCmdValue(ULONG Address, ULONG Value);
void LoadGrf(PSTR FileName);

void GetMsgStringTablePtr(ULONG StringID, PVOID* PtrBuffer);

// SCENEBASE
bool CWPushText(ULONG Color, PSTR String);
bool CWPushFormattedText(ULONG Color, PSTR Format, ...);

ULONG GetNetBase();

// GAMEBASE
bool ExecuteParser(USHORT Op, PVOID Buffer);
void SetConditionIcon(USHORT TypeID, UCHAR Flag);


#endif	// #ifndef CLIENT_H
