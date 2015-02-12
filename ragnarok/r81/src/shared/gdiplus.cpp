//! @file   gdiplus.cpp
#pragma comment(lib, "Gdiplus.lib")

#include "gdiplus.h"
#include <wchar.h>


namespace Gdiplus
{
//! Variables
ULONG_PTR GpToken = 0;

//! Initialize
void Startup()
{
    if (GpToken == 0)
    {
        GdiplusStartupInput GpStartupInput;
        GdiplusStartup(&GpToken, &GpStartupInput, NULL);
    }
}

//! Finalize
void Shutdown()
{
    if (GpToken)
        GdiplusShutdown(GpToken);
}

//!
bool GetEncoderCLSID(const wchar_t* MimeType, CLSID* Clsid)
{
    UINT NumEncoders=0, Size=0;
    GetImageEncodersSize(&NumEncoders, &Size);

    if (Size == 0)
        return false;

    bool result = false;
    ImageCodecInfo* Encoders = NULL;

    try
    {
        Encoders = new ImageCodecInfo[Size];

        for (UINT i=0; i<NumEncoders; i++)
        {
            if (wcscmp(Encoders[i].MimeType, MimeType) == 0)
            {
                *Clsid = Encoders[i].Clsid;
                result = true;
            }
        }

        delete [] Encoders;
    }
    catch (...)
    {
        if (Encoders)
            delete [] Encoders;
        throw;
    }

    return result;
}


};
