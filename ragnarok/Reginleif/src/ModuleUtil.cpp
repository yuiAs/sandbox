#include "Module.h"
#include "Client.h"
#include "WinAdvApi.h"
#include <shlwapi.h>
#include <wchar.h>
#ifdef WIN32_LEAN_AND_MEAN
#include <comdef.h>
#endif
#include <gdiplus.h>

// C4996: This function or variable may be unsafe.
#pragma warning(disable: 4996)

//------------------------------------------------------------------------------
// namespace Gdiplus
//------------------------------------------------------------------------------
namespace Gdiplus
{
//------------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------------
ULONG_PTR GpToken = 0;

//------------------------------------------------------------------------------
//  Function:   Gdiplus::Startup
//  Notes   :   GDI+èâä˙âª
//------------------------------------------------------------------------------
void Startup()
{
    GdiplusStartupInput GpStartupInput;
    GdiplusStartup(&GpToken, &GpStartupInput, NULL);
}

//------------------------------------------------------------------------------
//  Function:   Gdiplus::Shutdown
//  Notes   :   GDI+äJï˙
//------------------------------------------------------------------------------
void Shutdown()
{
    if (GpToken)
        GdiplusShutdown(GpToken);
}

//------------------------------------------------------------------------------
//  Function:   Gdiplus::GetEncoderCLSID
//  Notes   :   MIMEÇ©ÇÁCLSIDÇéÊìæÇ∑ÇÈÅB
//------------------------------------------------------------------------------
BOOL GetEncoderCLSID(CONST PWSTR MimeType, CLSID* Clsid)
{
    UINT NumEncoders=0, Size=0;

    GetImageEncodersSize(&NumEncoders, &Size);
    if (Size == 0)
        return FALSE;

    BOOL Result = FALSE;

    try
    {
        ImageCodecInfo* Encoders = new ImageCodecInfo[Size];
        GetImageEncoders(NumEncoders, Size, Encoders);

        for (UINT i=0; i<NumEncoders; i++)
        {
            if (wcscmp(Encoders[i].MimeType, MimeType) == 0)
            {
                *Clsid = Encoders[i].Clsid;
                Result = true;
                break;
            }
        }

        delete [] Encoders;
    }
    catch (...)
    {
    }

    return Result;
}

};  // Gdiplus

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------
void OutputBMP(Gdiplus::Bitmap* GpBmp);
void OutputJPEG(Gdiplus::Bitmap* GpBmp);
void OutputPNG(Gdiplus::Bitmap* GpBmp);

//------------------------------------------------------------------------------
//  Function:   APCScreenShot
//  Notes   :   
//------------------------------------------------------------------------------
void WINAPI APCScreenShot(ULONG_PTR dwParam)
{
    if (g_Pref.SSType == 0)
        return;
    if (g_Ctx.IsFocus == FALSE)
        return;

    HWND Wnd = g_Ctx.Wnd;

    RECT Rect;
    GetClientRect(Wnd, &Rect);

    HDC WndDC = GetDC(Wnd);
    HDC MemDC = CreateCompatibleDC(WndDC);
    HBITMAP Bitmap = CreateCompatibleBitmap(WndDC, Rect.right, Rect.bottom);
    HGDIOBJ Object = SelectObject(MemDC, Bitmap);
    BitBlt(MemDC, 0, 0, Rect.right, Rect.bottom, WndDC, 0, 0, SRCCOPY);

    Gdiplus::Bitmap* GpBmp = new Gdiplus::Bitmap(Bitmap, NULL);
    if (GpBmp == NULL)
        return;

    SelectObject(MemDC, Object);
    DeleteObject(Bitmap);
    DeleteObject(MemDC);
    ReleaseDC(Wnd, WndDC);

    switch (g_Pref.SSType)
    {
        case 1:
            OutputJPEG(GpBmp);
            break;
        case 2:
            OutputBMP(GpBmp);
            break;
        case 3:
            OutputPNG(GpBmp);
            break;
    }

    delete GpBmp;
}

//------------------------------------------------------------------------------
//  Function:   MakeSSFileName
//  Notes   :   
//------------------------------------------------------------------------------
void MakeSSFileName(PWSTR FileName, SIZE_T FileNameLength, CONST PWSTR Ext)
{
    SYSTEMTIME LocalTm;
    GetLocalTime(&LocalTm);

    _snwprintf(FileName, FileNameLength-1, L"%s/%04d_%02d_%02d_%02d_%02d_%02d.%s",
                                           L"ScreenShot", LocalTm.wYear, LocalTm.wMonth, LocalTm.wDay, LocalTm.wHour, LocalTm.wMinute, LocalTm.wSecond, Ext);
}

//------------------------------------------------------------------------------
//  Function:   NoticeSSFileName
//  Notes   :   
//------------------------------------------------------------------------------
void NoticeSSFileName(PWSTR FileName)
{
    if (g_Pref.SSNotice)
    {
        WCHAR FullPathName[MAX_PATH];
        PathSearchAndQualifyW(FileName, FullPathName, MAX_PATH);
        ClDrawTextExW(COLOR_GREEN, MSG_SCREENSHOTW, FullPathName);
    }
    else
    {
        ClDrawTextExW(COLOR_NAME_NPC, MSG_SCREENSHOTW, FileName);
    }
}

//------------------------------------------------------------------------------
//  Function:   OutputBMP
//  Notes   :   
//------------------------------------------------------------------------------
void OutputBMP(Gdiplus::Bitmap* GpBmp)
{
     CLSID Encoder;

    if (Gdiplus::GetEncoderCLSID(L"image/bmp", &Encoder))
    {
        WCHAR FileName[MAX_PATH];
        MakeSSFileName(FileName, MAX_PATH, L"bmp");
        GpBmp->Save(FileName, &Encoder);
        NoticeSSFileName(FileName);
    }
}

//------------------------------------------------------------------------------
//  Function:   OutputJPEG
//  Notes   :   
//------------------------------------------------------------------------------
void OutputJPEG(Gdiplus::Bitmap* GpBmp)
{
    CLSID Encoder;

    if (Gdiplus::GetEncoderCLSID(L"image/jpeg", &Encoder))
    {
        Gdiplus::EncoderParameters EncoderParams;
        EncoderParams.Count = 1;
        EncoderParams.Parameter[0].Guid = Gdiplus::EncoderQuality;
        EncoderParams.Parameter[0].NumberOfValues = 1;
        EncoderParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
        EncoderParams.Parameter[0].Value = reinterpret_cast<LONG*>(&g_Pref.SSQuality);

        WCHAR FileName[MAX_PATH];
        MakeSSFileName(FileName, MAX_PATH, L"jpg");
        GpBmp->Save(FileName, &Encoder, &EncoderParams);
        NoticeSSFileName(FileName);
    }
}

//------------------------------------------------------------------------------
//  Function:   OutputPNG
//  Notes   :   
//------------------------------------------------------------------------------
void OutputPNG(Gdiplus::Bitmap* GpBmp)
{
    CLSID Encoder;

    if (Gdiplus::GetEncoderCLSID(L"image/png", &Encoder))
    {
        WCHAR FileName[MAX_PATH];
        MakeSSFileName(FileName, MAX_PATH, L"png");
        GpBmp->Save(FileName, &Encoder);
        NoticeSSFileName(FileName);
    }
}
