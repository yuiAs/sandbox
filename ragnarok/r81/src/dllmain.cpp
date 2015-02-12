//! @file   dllmain.cpp

#include "stdafx.h"
#include "module.h"


//! Call from DllMain on DLL_PROCESS_ATTACH
void DllAttach(HANDLE Module)
{
#if defined(_DLL)
    // 静的リンクでは使ってはいけない、らしい。
    DisableThreadLibraryCalls(reinterpret_cast<HMODULE>(Module));
#endif
    ModuleInit(Module);
}

//! Call from DllMain on DLL_PROCESS_DETACH
void DllDetach(HANDLE Module)
{
    ModuleDestroy(Module);
}

//! DllMain
int __stdcall DllMain(HANDLE Module, ULONG Reason, PVOID Reserved)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH: DllAttach(Module); break;
        case DLL_PROCESS_DETACH: DllDetach(Module); break;
    }

    return 1;
}

