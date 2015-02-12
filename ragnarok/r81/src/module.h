//! @file   module.h
#pragma once

#include "stdafx.h"


//! Control flags
#define CTRL_CURRENT_FOCUS          0x00000001
#define CTRL_PROCESS_LFH            0x00000002
#define CTRL_FIX_BM                 0x00000004
#define CTRL_FIX_IME                0x00000008
#define CTRL_AUTOSAVECHAT           0x00000010
#define CTRL_EXTEND_SS              0x00000020
#define CTRL_DISABLE_SCROLL         0x00000040
#define CTRL_TRUESIGHT              0x00000080
#define CTRL_DONOT_DIVIDE_DAMAGE    0x00000100
#define CTRL_OIDCHECK               0x00100000

//! Default command states
#define DEFCMD_WINDOW       0x00000001
#define DEFCMD_SHOPPING     0x00000002

//! Memory addresses
typedef enum CLIENT_ADDRESS
{
    ADDR_OWNAID             = 0,
    ADDR_GAME,
    ADDR_ACTOR,
    ADDR_SCENE,
    ADDR_PACKETLENGTH,
    ADDR_ZONEPROCEDURE,
    ADDR_HULLCINATION,
    ADDR_WS2RECV,
    ADDR_WS2SEND,
    AEIP_GETMESSAGE,
    AEIP_GETCLASSINDEXSELF,
    AEIP_GETSKILLNAME,
    AEIP_GETPACKETINSTANCE,
    AEIP_DRAWSTRING,
    ADDR_ENUM_MAX,
} CLIENT_ADDRESS;

//! Module context
typedef struct _MODULE_CONTEXT
{
    HWND Wnd;
    ULONG CtrlFlag;
    ULONG Addr[ADDR_ENUM_MAX];
} MODULE_CONTEXT, *PMODULE_CONTEXT;

//! Module preference
typedef struct _MODULE_PREFERENCE
{
    ULONG Threshold;
    ULONG CodePage;
    ULONG SSType;
    ULONG SSQuality;
    ULONG WhisperType;
    ULONG DefaultCmd;
} MODULE_PREFERENCE, *PMODULE_PREFERENCE;


//! Global variables
extern MODULE_CONTEXT Context;
extern MODULE_PREFERENCE Pref;

//! module.cpp
void ModuleInit(HANDLE Module);
void ModuleDestroy(HANDLE Module);
inline HANDLE GetCoreAPCThread();


//! -
typedef struct _CHATPACKET_LIST
{
    PVOID Ptr;
    FILETIME Time;
    ULONG Type;
    UCHAR Data[1];
} CHATPACKET_LIST, *PCHATPACKET_LIST;

//! module2.cpp
void CALLBACK APCExtendSS(ULONG_PTR dwParam);
void CALLBACK APCChatLog(ULONG_PTR dwParam);


//! intelligence.cpp
void AutoCorrect();


//! packet.cpp
namespace Packet
{
    void Init();
    void Destroy();
    void Attach();
};
