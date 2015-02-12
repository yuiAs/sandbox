#ifndef _MODULE_REGINLEIF_
#define _MODULE_REGINLEIF_

#include <windows.h>
#include "EnumModule.h"
#include "ModuleResource.h"

//------------------------------------------------------------------------------
// Define
//------------------------------------------------------------------------------
#define MOD_FILENAME    L"randgriz.dll"
#define MOD_PREFERENCE  L"./randgriz.ini"

//------------------------------------------------------------------------------
// Struct
//------------------------------------------------------------------------------

typedef struct _MODULE_CONTEXT
{
    HWND Wnd;
    ULONG WndThreadId;
    BOOL IsFocus;
    ULONG Addr[CLADDR_ENUM_MAX];
} MODULE_CONTEXT, *PMODULE_CONTEXT;

typedef struct _MODULE_PREFERENCE
{
    ULONG Threshold;
    ULONG SSType;
    ULONG SSQuality;
    BOOL SSNotice;
    BOOL BMFix;
    BOOL BMSpace;
    BOOL IMEFix;
    BOOL AutoSaveChat;
    ULONG WhisperType;
    BOOL DefCmd[CLCMD_ENUM_MAX];
} MODULE_PREFERENCE, *PMODULE_PREFERENCE;

//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------
extern MODULE_CONTEXT g_Ctx;
extern MODULE_PREFERENCE g_Pref;

//------------------------------------------------------------------------------
// Module.cpp
//------------------------------------------------------------------------------
void InitModule(HANDLE Module);
void CleanupModule();

//------------------------------------------------------------------------------
// ModuleUtil.cpp
//------------------------------------------------------------------------------
namespace Gdiplus
{
void Startup();
void Shutdown();
};
void WINAPI APCScreenShot(ULONG_PTR dwParam);


#endif  // _MODULE_REGINLEIF_
