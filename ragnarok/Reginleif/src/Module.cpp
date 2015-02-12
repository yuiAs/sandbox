#define INCL_WINSOCK_API_TYPEDEFS 1
#include <winsock2.h>
#include "Module.h"
#include "Client.h"
#include "Packet.h"
#include "WinAdvApi.h"
#include <process.h>

//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------
MODULE_CONTEXT g_Ctx;
MODULE_PREFERENCE g_Pref;

//------------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------------
HHOOK lcl_CallWndHook = NULL;
HHOOK lcl_KeybdHook = NULL;
HHOOK lcl_MouseHook = NULL;
uintptr_t lcl_CoreThread = 0;
Packet* lcl_Network = NULL;

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
HANDLE GetCoreThreadHandle();
unsigned int __stdcall CoreThread(PVOID Parameter);
void InitWinsock2();

//------------------------------------------------------------------------------
//  Function:   InitModule
//  Notes   :   DLL_PROCESS_ATTACHをトリガーに呼ばれる初期化処理
//------------------------------------------------------------------------------
void InitModule(HANDLE Module)
{
    RtlZeroMemory(&g_Ctx, sizeof(MODULE_CONTEXT));
    RtlZeroMemory(&g_Pref, sizeof(MODULE_PREFERENCE));

    // Load preference
    g_Pref.Threshold = GetPrivateProfileIntW(L"CORE", L"THRESHOLD", 0, MOD_PREFERENCE);
    g_Pref.SSType = GetPrivateProfileIntW(L"CORE", L"SCREENSHOT", 0, MOD_PREFERENCE);
    g_Pref.SSQuality = GetPrivateProfileIntW(L"CORE", L"SCREENSHOT_JPEG_QUALITY", 0, MOD_PREFERENCE);
    g_Pref.SSNotice = GetPrivateProfileIntW(L"CORE", L"SCREENSHOT_CLIENTLIKE", 0, MOD_PREFERENCE);
    g_Pref.BMFix = GetPrivateProfileIntW(L"CORE", L"BATTLEMODE_FIX", 0, MOD_PREFERENCE);
    g_Pref.BMSpace = GetPrivateProfileIntW(L"CORE", L"BATTLEMODE_SPACE", 0, MOD_PREFERENCE);
    g_Pref.AutoSaveChat = GetPrivateProfileIntW(L"CLIENT", L"AUTOSAVECHAT", 0, MOD_PREFERENCE);
    g_Pref.WhisperType = GetPrivateProfileIntW(L"CLIENT", L"WHISPER_TYPE", 0, MOD_PREFERENCE);
    g_Pref.DefCmd[CMD_WI] = GetPrivateProfileIntW(L"CLIENT", L"DEFAULT_WI", 0, MOD_PREFERENCE);
    g_Pref.DefCmd[CMD_SH] = GetPrivateProfileIntW(L"CLIENT", L"DEFAULT_SH", 0, MOD_PREFERENCE);

    // GlobalHook
    // ASProtect展開前にやっておかないとnProtectがブロックしてくる
    if ((g_Pref.SSType>0) || g_Pref.BMFix || g_Pref.BMSpace)
    {
        if (lcl_KeybdHook == NULL)
            lcl_KeybdHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, reinterpret_cast<HMODULE>(Module), 0);
    }

    // LocalHook
   // if ((g_Pref.SSType>0) || g_Pref.BMFix || g_Pref.BMSpace)
    {
        if (lcl_CallWndHook == NULL)
            lcl_CallWndHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, NULL, GetCurrentThreadId());
    }

    DbgPrintW(_L("lcl_KeybdHook=%08X\n"), lcl_KeybdHook);
    DbgPrintW(_L("lcl_CallWndHook=%08X\n"), lcl_CallWndHook);
}

//------------------------------------------------------------------------------
//  Function:   InitModuleAfterCreateWnd
//  Notes   :   WM_CREATE直後のWM_SETFOCUSをトリガーに呼ばれる初期化処理
//------------------------------------------------------------------------------
void InitModuleAfterCreateWnd()
{
    Gdiplus::Startup();
    RunInvestigator();

    if (lcl_Network == NULL)
    {
        lcl_Network = new Packet;
        lcl_Network->Init();
    }

    // APC待機用スレッドを起動
    if (lcl_CoreThread == NULL)
        lcl_CoreThread = _beginthreadex(NULL, 0, CoreThread, NULL, 0, NULL);

    DbgPrintW(_L("lcl_CoreThread=%08X\n"), lcl_CoreThread);
}

//------------------------------------------------------------------------------
//  Function:   CleanupModule
//  Notes   :   
//------------------------------------------------------------------------------
void CleanupModule()
{
    if (lcl_KeybdHook)
        UnhookWindowsHookEx(lcl_KeybdHook);
    if (lcl_CallWndHook)
        UnhookWindowsHookEx(lcl_CallWndHook);
    if (lcl_Network)
        delete lcl_Network;

    TerminateThread(GetCoreThreadHandle(), 0);
    CloseHandle(GetCoreThreadHandle());

    Gdiplus::Shutdown();
}


////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   CallWndProc
//  Notes   :   WH_CALLWNDPROC
//------------------------------------------------------------------------------
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
        return CallNextHookEx(lcl_CallWndHook, nCode, wParam, lParam);

    if (nCode == HC_ACTION)
    {
        PCWPSTRUCT cwp = reinterpret_cast<PCWPSTRUCT>(lParam);
        switch (cwp->message)
        {
            case WM_SETFOCUS:
                DbgPrintW(_L("WM_SETFOCUS Wnd=%08X\n"), cwp->hwnd);
                if (g_Ctx.Wnd == 0)
                {
                    g_Ctx.Wnd = cwp->hwnd;
                    g_Ctx.WndThreadId = GetWindowThreadProcessId(cwp->hwnd, NULL);
                    InitModuleAfterCreateWnd();
                }
                if (g_Ctx.Wnd == cwp->hwnd)
                    g_Ctx.IsFocus = TRUE;
                break;

            case WM_KILLFOCUS:
                DbgPrintW(_L("WM_KILLFOCUS Wnd=%08X\n"), cwp->hwnd);
                if (g_Ctx.Wnd == cwp->hwnd)
                    g_Ctx.IsFocus = FALSE;
                break;
        }
    }

    return CallNextHookEx(lcl_CallWndHook, nCode, wParam, lParam);
}

//------------------------------------------------------------------------------
//  Function:   LowLevelKeyboardProc
//  Notes   :   WH_KEYBOARD_LL
//------------------------------------------------------------------------------
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
        return CallNextHookEx(lcl_KeybdHook, nCode, wParam, lParam);

    if (nCode == HC_ACTION)
    {
        if (g_Ctx.IsFocus == FALSE)
            return CallNextHookEx(lcl_KeybdHook, nCode, wParam, lParam);

        KBDLLHOOKSTRUCT* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        switch (kb->vkCode)
        {
            case VK_SNAPSHOT:
                if (g_Pref.SSType > 0)
                {
                    if (wParam == WM_KEYUP)
                        QueueUserAPC(APCScreenShot, GetCoreThreadHandle(), kb->vkCode);
                    return TRUE;
                }
                break;

            case VK_SCROLL:
                if (g_Pref.SSType > 0)
                    return TRUE;
                break;

            case VK_SPACE:
                if (g_Pref.BMSpace)
                {
                    if (ClCheckCMDState(VA_STATE_BM, 1) && ClCheckCMDState(VA_STATE_BMINPUT, 0))
                    {
                        if (wParam == WM_KEYUP)
                            ClSetCMDState(VA_STATE_BMINPUT, 1);
                        return TRUE;
                    }
                }
                break;
        }
    }

    return CallNextHookEx(lcl_KeybdHook, nCode, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   GetCoreThreadHandle
//  Notes   :   
//------------------------------------------------------------------------------
HANDLE GetCoreThreadHandle()
{
    return reinterpret_cast<HANDLE>(lcl_CoreThread);
}

//------------------------------------------------------------------------------
//  Function:   CoreThread
//  Notes   :   APC待機用スレッド
//------------------------------------------------------------------------------
unsigned int __stdcall CoreThread(PVOID Parameter)
{
    DbgPrintW(_L("CoreThread=%08X\n"), GetCurrentThreadId());

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
    InitWinsock2();     // ループが発生する都合上こちら
    SwitchToThread();   // とりあえず区切りという意味でCall

    while (1)
        SleepEx(INFINITE, TRUE);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------------
LPFN_RECV lcl_Recv = NULL;
LPFN_SEND lcl_Send = NULL;

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------
int WSAAPI IntrnlRecv(int a, char* b, int c, int d);
//int WSAAPI IntrnlSend(int a, const char* b, int c, int d);

//------------------------------------------------------------------------------
//  Function:   InitWinsock2
//  Notes   :   クライアント処理の都合上whileループさせてます
//------------------------------------------------------------------------------
void InitWinsock2()
{
    if (ULONG Addr = g_Ctx.Addr[VA_WS2RECV])
    {
        while (*reinterpret_cast<ULONG*>(Addr)==0)
            SleepEx(100, TRUE);

        lcl_Recv = reinterpret_cast<LPFN_RECV>(InterlockedExchangePointer(reinterpret_cast<PVOID>(Addr), IntrnlRecv));
    }

    DbgPrintW(_L("WS2RECV %08X->%08X [%08X]\n"), lcl_Recv, IntrnlRecv, g_Ctx.Addr[VA_WS2RECV]);
}

int WSAAPI IntrnlRecv(int a, char* b, int c, int d)
{
    int Result = lcl_Recv(a, b, c, d);
    if (Result > 0)
        Result = lcl_Network->Recv(b, Result);

    return Result;
}
