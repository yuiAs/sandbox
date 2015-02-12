//! @file   module2.cpp

#include "stdafx.h"
#include <shlwapi.h>
#include "module.h"


////////////////////////////////////////////////////////////////////////////////

//! Forward declarations
void OutputBMP(Gdiplus::Bitmap* gpbmp);

//! Calling the QueueUserAPC.
void CALLBACK APCExtendSS(ULONG_PTR dwParam)
{
    if (Context.Wnd == NULL)
        return;

    HWND wnd = Context.Wnd;

    RECT rect;
    GetClientRect(wnd, &rect);

    HDC dcWnd = GetDC(wnd);
    HDC dcMem = CreateCompatibleDC(dcWnd);
    HBITMAP bitmap = CreateCompatibleBitmap(dcMem, rect.right, rect.bottom);
    HGDIOBJ object = SelectObject(dcMem, bitmap);
    BitBlt(dcMem, 0, 0, rect.right, rect.bottom, dcWnd, 0, 0, SRCCOPY);

    Gdiplus::Bitmap* gpbmp = new Gdiplus::Bitmap(bitmap, NULL);

    SelectObject(dcMem, object);
    DeleteObject(bitmap);
    DeleteObject(dcMem);
    ReleaseDC(wnd, dcWnd);

    if (gpbmp == NULL)
        return;

#if 1
    OutputBMP(gpbmp);
#else
    switch (Pref.SSType)
    {
        case 1: OutputBMP(gpbmp); break;
        case 2: OutputJPEG(gpbmp); break;
        case 3: OutputPNG(gpbmp); break;
    }
#endif

    delete gpbmp;
}

//! Make Screenshot filename.
void MakeSSFileName(wchar_t* FileName, size_t FileNameLength, wchar_t* Ext)
{
    SYSTEMTIME tmLocal;
    GetLocalTime(&tmLocal);

    _snwprintf(FileName, FileNameLength-1, L"%s/%04d_%02d_%02d_%02d_%02d_%02d.%s",
                                           L"ScreenShot", tmLocal.wYear, tmLocal.wMonth, tmLocal.wDay, tmLocal.wHour, tmLocal.wMinute, tmLocal.wSecond, Ext);
}

//! Save Bitmap format.
void OutputBMP(Gdiplus::Bitmap* gpbmp)
{
    CLSID Encoder;

    if (Gdiplus::GetEncoderCLSID(L"image/bmp", &Encoder))
    {
        wchar_t filename[MAX_PATH];
        MakeSSFileName(filename, MAX_PATH, L"bmp");
        gpbmp->Save(filename, &Encoder);
    }
}

////////////////////////////////////////////////////////////////////////////////

//! Forward declarations

//! -
void CALLBACK APCChatLog(ULONG_PTR dwParam)
{
    PCHATPACKET_LIST item = reinterpret_cast<PCHATPACKET_LIST>(dwParam);
    HeapFree(GetProcessHeap(), 0, item);
}
