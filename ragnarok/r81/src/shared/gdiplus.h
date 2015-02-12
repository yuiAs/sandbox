//! @file   gdiplus.h
#pragma once

#if !defined(_WIN32_DCOM)
#define _WIN32_DCOM
#endif

#include <objbase.h>
#include <Gdiplus.h>


namespace Gdiplus
{
    void Startup();
    void Shutdown();
    bool GetEncoderCLSID(const wchar_t* MimeType, CLSID* Clsid);
};
