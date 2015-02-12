#ifndef INTERNL_H
#define INTERNL_H

#include "win32.h"
#include "debug.h"

////////////////////////////////////////////////////////////////////////////////

// Static strings

static const wchar_t* CONFNAME = _CRT_WIDE("./randgriz.ini");

////////////////////////////////////////////////////////////////////////////////

#include "intrnlEnum.h"

// Session Data

typedef struct _SESSION_TBL
{
	bool ConnFirst;
	ULONG AID;
	USHORT EnArms;
	ULONG EnArmsCount;
	bool NoDivDmg;
	bool TrueSight;
	ULONG TrueSightDelay;
} SESSION_TBL, *PTR_SESSION_TBL;

// Internal Table

typedef struct _INTRNL_TBL
{
	ULONG Threshold;
	bool DisableSend;
	bool DisableEXMsg;
	bool ClCmd[INTRNLCLCMD_MAX];
	ULONG Addr[INTRNLADDR_MAX];
	SESSION_TBL Session;
} INTRNL_TBL, *PTR_INTRNL_TBL;

extern INTRNL_TBL g_Intrnl;

////////////////////////////////////////////////////////////////////////////////

// module.cpp

void InitModule(HANDLE Module);
void DestroyModule();

// moduleNetwork.cpp

void InitNetwork();
void DestroyNetwork();

////////////////////////////////////////////////////////////////////////////////

// Load Config

inline bool ConfIsEnable(wchar_t* AppName, wchar_t* KeyName)
{
	return (GetPrivateProfileInt(AppName, KeyName, 0, CONFNAME)==1);
}

inline int ConfGetVal(wchar_t* AppName, wchar_t* KeyName, int Default)
{
	return GetPrivateProfileInt(AppName, KeyName, Default, CONFNAME);
}


#endif	// #ifndef INTERNL_H
