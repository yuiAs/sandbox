#include "Module.h"
#include "Client.h"
#include "WinAdvApi.h"
#include <wchar.h>
#include <stdio.h>

// C4996: This function or variable may be unsafe.
#pragma warning(disable: 4996)

//------------------------------------------------------------------------------
// Defined
//------------------------------------------------------------------------------
#ifndef DRAWTEXT_MAX_LENGTH
#define DRAWTEXT_MAX_LENGTH 104
#endif

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   ClGetMsgStr
//  Notes   :   cdecl calling convention
//------------------------------------------------------------------------------
PSTR ClGetMsgStr(ULONG MsgID)
{
    ULONG Function = g_Ctx.Addr[VC_GETMSGSTR];

    __asm
    {
        push    MsgID
        call    dword ptr [Function]
        add     esp, 4
    }
}

//------------------------------------------------------------------------------
//  Function:   ClGetActorClassName
//  Notes   :   
//------------------------------------------------------------------------------
PSTR ClGetActorClassName(ULONG Index)
{
    ULONG Table = g_Ctx.Addr[VA_ACTORCLASSNAMETABLE];

    __asm
    {
        mov     esi, Table
        mov     edi, Index
        mov     eax, [esi+edi*4]
    }
}

//------------------------------------------------------------------------------
//  Function:   ClGetOwnClassIndex
//  Notes   :   
//------------------------------------------------------------------------------
ULONG ClGetOwnClassIndex()
{
    ULONG Function = g_Ctx.Addr[VC_GETOWNCLASSINDEX];
    ULONG PtrThis = g_Ctx.Addr[VA_ACTORBASE];

    __asm
    {
        mov     ecx, PtrThis
        call    dword ptr [Function]
    }
}

//------------------------------------------------------------------------------
//  Function:   ClGetSkillName
//  Notes   :   fastcall calling convertion
//------------------------------------------------------------------------------
PSTR ClGetSkillName(ULONG SkillID)
{
    CLSKILLINFO_CONTAINER SkInfo;
    RtlZeroMemory(&SkInfo, sizeof(CLSKILLINFO_CONTAINER));
    SkInfo._Unk1 = 1;
    SkInfo.SkillID = SkillID;

    ULONG Function = g_Ctx.Addr[VC_GETSKILLNAME];

    __asm
    {
        lea     ecx, SkInfo
        call    dword ptr [Function]
    }
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   ClDrawTextInternal
//  Notes   :   
//------------------------------------------------------------------------------
void ClDrawTextInternal(PSTR String, ULONG Color, ULONG Mode)
{
    ULONG Function = g_Ctx.Addr[VC_DRAWTEXT];
    ULONG PtrThis = g_Ctx.Addr[VA_SCENEBASE];

    __asm
    {
        push    0
        push    0
        push    Color
        push    String
        push    Mode
        mov     ecx, PtrThis
        call    dword ptr [Function]
    }
}

//------------------------------------------------------------------------------
//  Function:   ClDrawTextA
//  Notes   :   
//------------------------------------------------------------------------------
void ClDrawTextA(PSTR String, ULONG Color)
{
    if (strlen(String) < DRAWTEXT_MAX_LENGTH)
        ClDrawTextInternal(String, Color, 1);
}

//------------------------------------------------------------------------------
//  Function:   ClDrawTextExA
//  Notes   :   
//------------------------------------------------------------------------------
void ClDrawTextExA(ULONG Color, PSTR Format, ...)
{
#ifdef CLDRAWTEXT_HEAPALLOC
    char* Buf = reinterpret_cast<char*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DRAWTEXT_MAX_LENGTH));
#else
    char Buf[DRAWTEXT_MAX_LENGTH];
    RtlZeroMemory(Buf, sizeof(Buf));
#endif

    va_list va;
    va_start(va, Format);

    int Result = _vsnprintf(Buf, DRAWTEXT_MAX_LENGTH-1, Format, va);
    if (Result != -1)
        ClDrawTextInternal(Buf, Color, 1);

    va_end(va);

#ifdef CLDRAWTEXT_HEAPALLOC
    HeapFree(GetProcessHeap(), 0, Buf);
#endif
}

//------------------------------------------------------------------------------
//  Function:   ClDrawTextW
//  Notes   :   
//------------------------------------------------------------------------------
void ClDrawTextW(PWSTR String, ULONG Color)
{
    if (wcslen(String) < DRAWTEXT_MAX_LENGTH)
    {
        ANSI_STRING AnsiString;
        CreateAnsiStringW(&AnsiString, String);
        ClDrawTextInternal(AnsiString.Buffer, Color, 1);
        RtlFreeAnsiString(&AnsiString);
    }
}

//------------------------------------------------------------------------------
//  Function:   ClDrawTextExW
//  Notes   :   
//------------------------------------------------------------------------------
void ClDrawTextExW(ULONG Color, PWSTR Format, ...)
{
#ifdef CLDRAWTEXT_HEAPALLOC
    WCHAR* Buf = reinterpret_cast<char*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DRAWTEXT_MAX_LENGTH<<1));
#else
    WCHAR Buf[DRAWTEXT_MAX_LENGTH];
    RtlZeroMemory(Buf, sizeof(Buf));
#endif

    va_list va;
    va_start(va, Format);

    int Result = _vsnwprintf(Buf, DRAWTEXT_MAX_LENGTH-1, Format, va);
    if (Result != -1)
    {
        ANSI_STRING AnsiString;
        CreateAnsiStringW(&AnsiString, Buf);
        ClDrawTextInternal(AnsiString.Buffer, Color, 1);
        RtlFreeAnsiString(&AnsiString);
    }

    va_end(va);

#ifdef CLDRAWTEXT_HEAPALLOC
    HeapFree(GetProcessHeap(), 0, Buf);
#endif
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   ClGetNetworkInstance
//  Notes   :   タイミングによってはNULLが返る。
//              たぶんlogin.rswのロード前。
//------------------------------------------------------------------------------
ULONG ClGetNetworkInstance()
{
    ULONG Function = g_Ctx.Addr[VC_GETNETBASE];

    __asm
    {
        call    dword ptr [Function]
    }
}

//------------------------------------------------------------------------------
//  Function:   FindPacketLength
//  Notes   :   再帰系
//              クライアントもLength=2をデフォルトで返しているので踏襲
//------------------------------------------------------------------------------
int FindPacketLength(PPACKETLENGTH_CONTAINER Container, ULONG Op)
{
    if (Container)
    {
        if (Container->Op == Op)
            return Container->Length;
        else
        {
            if (Container->Op > Op)
                return FindPacketLength(Container->Left, Op);
            else
                return FindPacketLength(Container->Right, Op);
        }
    }

    return 2;
}

//------------------------------------------------------------------------------
//  Function:   ClGetPacketLength
//  Notes   :   
//------------------------------------------------------------------------------
int ClGetPacketLength(PVOID Buffer)
{
    PPACKETLENGTH_CONTAINER p = reinterpret_cast<PPACKETLENGTH_CONTAINER>(g_Ctx.Addr[VA_PACKETLENGTHCONTAINER])->Parent;
    USHORT Op = *reinterpret_cast<USHORT*>(Buffer);

    while ((p!=NULL) && (p->Op!=Op))
        p = (Op < p->Op) ? p->Left : p->Right;

    if ((p==NULL) || (p->Length==0))
        return 2;
    else
        return p->Length;
}

#if 0   // この処理だとESP周りで整合性がおかしくなるらしいので要確認
//------------------------------------------------------------------------------
//  Function:   ClGetPacketLength
//  Notes   :   fastcall calling convertion
//------------------------------------------------------------------------------
LONG ClGetPacketLength(PVOID Buffer)
{
    ULONG Function = g_Ctx.Addr[VC_GETPACKETLENGTH];

    __asm
    {
        mov     ecx, Buffer
        call    dword ptr [Function]
    }
}
#endif

//------------------------------------------------------------------------------
//  Function:   GetZoneProcedure
//  Notes   :   
//------------------------------------------------------------------------------
ULONG GetZoneProcedure(USHORT Op)
{
    ULONG Table = g_Ctx.Addr[VA_ZONETABLE];

    __asm
    {
        mov     esi, Table
        movsx   eax, word ptr [Op]
        add     eax, 0FFFFFF8Dh
        mov     edi, [esi+eax*4]
        xor     eax, eax
        mov     al, byte ptr [edi+9]
        cmp     eax, 0E8h
        mov     eax, 0
        jne     _DO_NOT_MATCH
        mov     edx, [edi+0Ah]
        lea     eax, [edi+edx+0Eh]
    _DO_NOT_MATCH:
    }
}

//------------------------------------------------------------------------------
//  Function:   ClCallZoneProcedure
//  Notes   :   
//------------------------------------------------------------------------------
void ClCallZoneProcedure(USHORT Op, PVOID Buffer)
{
    ULONG Function = GetZoneProcedure(Op);
    if (Function == 0)
        return;

    ULONG PtrThis = g_Ctx.Addr[VA_GAMEBASE];

    __try
    {
        __asm
        {
            mov     eax, PtrThis
            mov     ecx, [eax+4]
            push    Buffer
            call    dword ptr [Function]
        }
    }
    __except (OutputExceptionInfo(GetExceptionInformation()))
    {
        DbgPrintW(_L("EXCEPTION ClCallZoneProcedure OP=%04X BUFFER=%08X\n"), Op, Buffer);
    }
}

//------------------------------------------------------------------------------
//  Function:   ClSetEffectState
//  Notes   :   
//------------------------------------------------------------------------------
void ClSetEffectState(ULONG ObjectID, USHORT StateType, UCHAR Flag)
{
    UCHAR Buffer[9];
    *reinterpret_cast<USHORT*>(Buffer) = PACKET_ZC_EFFECT_STATE;
    *reinterpret_cast<USHORT*>(Buffer+2) = StateType;
    *reinterpret_cast<ULONG*>(Buffer+4) = ObjectID;
    *reinterpret_cast<UCHAR*>(Buffer+8) = Flag;

    ClCallZoneProcedure(PACKET_ZC_EFFECT_STATE, Buffer);
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   ClSetCMDState
//  Notes   :   
//------------------------------------------------------------------------------
void ClSetCMDState(ULONG Index, LONG Value)
{
    if (ULONG Addr = g_Ctx.Addr[Index])
        InterlockedExchange(reinterpret_cast<LONG*>(Addr), Value);
}

//------------------------------------------------------------------------------
//  Function:   ClGetCMDState
//  Notes   :   
//------------------------------------------------------------------------------
LONG ClGetCMDState(ULONG Index)
{
    LONG Result = -1;

    if (ULONG Addr = g_Ctx.Addr[Index])
        Result = *reinterpret_cast<LONG*>(Addr);

    return Result;
}

//------------------------------------------------------------------------------
//  Function:   ClCheckCMDState
//  Notes   :   
//------------------------------------------------------------------------------
BOOL ClCheckCMDState(ULONG Index, LONG Value)
{
    LONG Current = ClGetCMDState(Index);
    if (Current == -1)
        return FALSE;
    else
        return (Current==Value) ? TRUE : FALSE;

}
