//! @file   module.cpp

#include "stdafx.h"
#include "module.h"


//! Global variables
MODULE_CONTEXT Context;
MODULE_PREFERENCE Pref;

//! Variables
HHOOK hkCallWnd = NULL;
HHOOK hkLLKeyboard = NULL;
uintptr_t idCoreThread = 0;

//! Forward declarations
LRESULT CALLBACK CallWndProc(int Code, WPARAM wp, LPARAM lp);
LRESULT CALLBACK LowLevelKeyboardProc(int Code, WPARAM wp, LPARAM lp);


//! Initialize
void ModuleInit(HANDLE Module)
{
    DbgPrintW(L"ModuleInit TICK=%08X\n", GetTickCount());

    RtlZeroMemory(&Context, sizeof(MODULE_CONTEXT));
    RtlZeroMemory(&Pref, sizeof(MODULE_PREFERENCE));

    Pref.Threshold = 0x000C0000;

    // Hook Low-level keyboard.
    // ASProtect ìWäJëOÇ…Ç‚ÇÁÇ»Ç¢Ç∆ nProtect Ç™é~ÇﬂÇ…óàÇÈÅB
    if (1)
    {
        if (hkLLKeyboard == NULL)
            hkLLKeyboard = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, reinterpret_cast<HMODULE>(Module), 0);
        DbgPrintW(L"WH_KEYBOARD_LL %08X\n", hkLLKeyboard);
    }

    // Hook Call window procedure.
    if (1)
    {
        if (hkCallWnd == NULL)
            hkCallWnd = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, NULL, GetCurrentThreadId());
        DbgPrintW(L"WH_CALLWNDPROC %08X\n", hkCallWnd);
    }
}

//! Finalize
void ModuleDestroy(HANDLE Module)
{
    if (hkLLKeyboard)
        UnhookWindowsHookEx(hkLLKeyboard);
    if (hkCallWnd)
        UnhookWindowsHookEx(hkCallWnd);

    if (idCoreThread > 0)
    {
        HANDLE th = GetCoreAPCThread();
        TerminateThread(th, 0);
        CloseHandle(th);
    }

    Gdiplus::Shutdown();
}

//! Core APCThread
unsigned __stdcall CoreThread(PVOID Parameter)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
    Packet::Attach();
    SwitchToThread();

    while (1)
        SleepEx(INFINITE, TRUE);

    return 0;
}

//! 
inline HANDLE GetCoreAPCThread()
{
    return reinterpret_cast<HANDLE>(idCoreThread);
}

//! Trigger off WM_SETFOCUS after WM_CREATE.
void InitAfterCreateWnd()
{
    Gdiplus::Startup();
    AutoCorrect();
    Packet::Init();

    if (idCoreThread == 0)
        idCoreThread = _beginthreadex(NULL, 0, CoreThread, NULL, 0, NULL);
}

//! WH_CALLWNDPROC
LRESULT CALLBACK CallWndProc(int Code, WPARAM wp, LPARAM lp)
{
    if (Code < 0)
        return CallNextHookEx(hkCallWnd, Code, wp, lp);

    if (Code == HC_ACTION)
    {
        PCWPSTRUCT cwp = reinterpret_cast<PCWPSTRUCT>(lp);
        switch (cwp->message)
        {
            case WM_SETFOCUS:
                if (Context.Wnd == NULL)
                {
                    Context.Wnd = cwp->hwnd;
                    InitAfterCreateWnd();
                }
                if (Context.Wnd == cwp->hwnd)
                    Context.CtrlFlag |= CTRL_CURRENT_FOCUS;
                break;
            case WM_KILLFOCUS:
                if (Context.Wnd == cwp->hwnd)
                    Context.CtrlFlag &= (~CTRL_CURRENT_FOCUS);
                break;
        }
    }

    return CallNextHookEx(hkCallWnd, Code, wp, lp);
}

//! WH_KEYBOARD_LL
LRESULT CALLBACK LowLevelKeyboardProc(int Code, WPARAM wp, LPARAM lp)
{
    if (Code < 0)
        return CallNextHookEx(hkLLKeyboard, Code, wp, lp);

    if (Code == HC_ACTION)
    {
        if ((Context.CtrlFlag&CTRL_CURRENT_FOCUS) == 0)
            return CallNextHookEx(hkLLKeyboard, Code, wp, lp);

        KBDLLHOOKSTRUCT* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lp);

        switch (kb->vkCode)
        {
            case VK_SNAPSHOT:
                if (Context.CtrlFlag & CTRL_EXTEND_SS)
                {
                    if (wp == WM_KEYUP)
                        QueueUserAPC(APCExtendSS, reinterpret_cast<HANDLE>(idCoreThread), kb->vkCode);
                    return TRUE;
                }
                break;

            case VK_SCROLL:
                if (Context.CtrlFlag & CTRL_DISABLE_SCROLL)
                    return TRUE;
                break;
        }
    }

    return CallNextHookEx(hkLLKeyboard, Code, wp, lp);
}
