#include "Module.h"
#include "Client.h"
#include "WinAdvApi.h"

//------------------------------------------------------------------------------
// Define
//------------------------------------------------------------------------------
#ifndef WS2API_SEARCH_RANGE
#define WS2API_SEARCH_RANGE 0x0100
#endif

#ifndef PACKETLENGTHCONTAINER_SEARCH_RANGE
#define PACKETLENGTHCONTAINER_SEARCH_RANGE  0x00030000
#endif

//------------------------------------------------------------------------------
// Enum
//------------------------------------------------------------------------------
enum Instruction
{
    OP_39=0,
    OP_3D,
    OP_85,
    OP_8A,
    OP_8A88,
    OP_8B,
    OP_8B0D,
    OP_8B15,
    OP_8B49,
    OP_8B88,
    OP_A1,
    OP_B9,
    OP_C7,
    OP_C745,
    OP_E8,
    OP_E9,
    OP_FF,
    OP_FF24,
    OP_0F84,
    INSTRUCTION_ENUM_MAX,
};

//------------------------------------------------------------------------------
// Local variables
//------------------------------------------------------------------------------
ULONG lcl_PEDataSection = 2;

//------------------------------------------------------------------------------
// Static variables
//------------------------------------------------------------------------------
#include <pshpack1.h>
UCHAR Instructions[][INSTRUCTION_ENUM_MAX] = {
    { 0x39, },                          // cmp  r/m32, r32
    { 0x3D, /*id*/ },                   // cmp  eax, imm32
    { 0x85, },                          // test r/m32, r32
    { 0x8A, },                          // mov  r8, r/m8
    { 0x8A, 0x88, /*id*/ },             // mov  cl, [eax+disp32]
    { 0x8B, },                          // mov  r32, r/m32
    { 0x8B, 0x0D, /*cd*/ },             // mov  eax, r/m32
    { 0x8B, 0x15, /*cd*/ },             // mov  edx, r/m32
    { 0x8B, 0x49, /*ib*/ },             // mov  ecx, [ecx+disp8]
    { 0x8B, 0x88, /*ib*/ },             // mov  ecx, [eax+disp8]
    { 0xA1, /*cd*/ },                   // mov  eax, moffs32
    { 0xB9, /*cd*/ },                   // mov  ecx, imm32
    { 0xC7, },                          // mov  r/m32, imm32
    { 0xC7, 0x45, /*ib cd*/ },          // mov  [ebp+disp8], imm32
    { 0xE8, /*cd*/ },                   // call rel32
    { 0xE9, /*cd*/ },                   // jmp  rel32
    { 0xFF, },                          // jmp  r/m32, m16:32
    { 0xFF, 0x24, /*SIB cd*/ },         // jmp  m16:32
    { 0x0F, 0x84, /*cd*/ },             // je   rel32
};
#include <poppack.h>

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   CompareOp
//  Notes   :   
//------------------------------------------------------------------------------
inline BOOL CompareOp(ULONG Source, enum Instruction Index, SIZE_T Length)
{
    return RtlEqualMemory(Instructions[Index], reinterpret_cast<PVOID>(Source), Length);
}

//------------------------------------------------------------------------------
//  Function:   GetEIPEx
//  Notes   :   
//------------------------------------------------------------------------------
BOOL GetEIPEx(ULONG Address, PULONG Result)
{
    if (CompareOp(Address, OP_E8, 1) == FALSE)
        return FALSE;
    else
    {
        ULONG Relative = *reinterpret_cast<ULONG*>(Address+1);
        *Result = Relative + Address + 5;
        return TRUE;
    }
}

//------------------------------------------------------------------------------
//  Function:   GetEIP
//  Notes   :   
//------------------------------------------------------------------------------
ULONG GetEIP(ULONG Address)
{
    ULONG Value = 0;
    GetEIPEx(Address, &Value);
    return Value;
}

//------------------------------------------------------------------------------
//  Function:   GetImageNTHeaders
//  Notes   :   
//------------------------------------------------------------------------------
PIMAGE_NT_HEADERS GetImageNTHeaders(HANDLE Module)
{
    PIMAGE_DOS_HEADER DOSHdr = reinterpret_cast<PIMAGE_DOS_HEADER>(Module);
    return reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<LONG>(DOSHdr)+DOSHdr->e_lfanew);
}

//------------------------------------------------------------------------------
//  Function:   GetPESection
//  Notes   :   
//------------------------------------------------------------------------------
VOID GetPESection(ULONG Offset, PULONG Base, PULONG Size)
{
    PIMAGE_NT_HEADERS NTHdr = GetImageNTHeaders(GetModuleHandle(NULL));
    PIMAGE_SECTION_HEADER SectHdr = reinterpret_cast<PIMAGE_SECTION_HEADER>(NTHdr+1) + Offset;

    if (Base)
        *Base = NTHdr->OptionalHeader.ImageBase + SectHdr->VirtualAddress;
    if (Size)
        *Size = SectHdr->Misc.VirtualSize;
}

//------------------------------------------------------------------------------
//  Function:   CompareBin
//  Notes   :   
//------------------------------------------------------------------------------
BOOL CompareBin(CONST PVOID Data, SIZE_T Length, LONG Index, ULONG Base, ULONG Size, PULONG Result)
{
    *Result = 0;

    if (Index != -1)
        GetPESection(Index, &Base, &Size);

    for (ULONG i=Base; i<(Base+Size)-Length; i++)
    {
        __try
        {
            if (RtlEqualMemory(reinterpret_cast<PVOID>(i), Data, Length))
            {
                *Result = i;
                __leave;
            }
        }
        __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
        {
            DbgPrintW(_L("ACCESS_VIOLATION %08X (CompareBin)\n"), i);
        }

        if (*Result)
            break;
    }

    return (*Result!=0);
}

//------------------------------------------------------------------------------
//  Function:   FindBinFirst
//  Notes   :   
//------------------------------------------------------------------------------
BOOL FindBinFirst(CONST PUCHAR Data, SIZE_T Length, PULONG Result)
{
    return CompareBin(reinterpret_cast<PVOID>(Data), Length, 0, 0, 0, Result);
}

//------------------------------------------------------------------------------
//  Function:   FindStrFirstA
//  Notes   :   
//------------------------------------------------------------------------------
BOOL FindStrFirstA(CONST PSTR String, PULONG Result)
{
    return CompareBin(reinterpret_cast<PVOID>(String), strlen(String)+1, lcl_PEDataSection, 0, 0, Result);
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   Investigator01
//  Notes   :   VA_ZONETABLE
//              VC_GETNETBASE
//------------------------------------------------------------------------------
void Investigator01()
{
    DbgPrintW(_L("Investigator01 Tick=%08X\n"), GetTickCount());
/*
  // 2007-11-13aSakexe
  // 0058DEA2
  8Drr********      lea     reg_1, [ebp-imm32]
  5r                push    reg_1
  E8********        call    rel32                   ; Network::GetInstance
  8BC8              mov     ecx, eax
  E8********        call    rel32
  0FBFC0            mov     eax, ax
  83C08D            add     eax, 0FFFFFF8Dh         ; -73h
  3D********        cmp     eax, imm32
  0F87********      ja      rel32
  FF2485********    jmp     [disp32+eax*4]
*/
    ULONG Pos = 0;
    UCHAR Pattern[] = {
        0x0F, 0xBF, 0xC0,
        0x83, 0xC0, 0x8D,
    };

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    // VA_ZONETABLE
    if (CompareOp(Pos+17, OP_FF24, 2))
    {
        ULONG Value = *reinterpret_cast<ULONG*>(Pos+20);
        g_Ctx.Addr[VA_ZONETABLE] = Value;
    }

    // VC_GETNETBASE
    if (ULONG Value = GetEIP(Pos-12))
        g_Ctx.Addr[VC_GETNETBASE] = Value;

    DbgPrintW(_L("VA_ZONETABLE=%08X\n"), g_Ctx.Addr[VA_ZONETABLE]);
    DbgPrintW(_L("VC_GETNETBASE=%08X\n"), g_Ctx.Addr[VC_GETNETBASE]);
}

//------------------------------------------------------------------------------
//  Function:   Investigator02
//  Notes   :   VA_WS2SEND
//              VA_WS2RECV
//------------------------------------------------------------------------------
void Investigator02()
{
    DbgPrintW(_L("Investigator02 Tick=%08X\n"), GetTickCount());

    ULONG Pos = 0;

     if (FindStrFirstA("recv", &Pos))
        DbgPrintW(_L("CodeString POS=%08X\n"), Pos);
    else
        return;

    UCHAR Pattern[] = {
        0x68, 0x00, 0x00, 0x00, 0x00,
    };
    RtlCopyMemory(Pattern+1, &Pos, sizeof(ULONG));

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    ULONG InRecv = 0;
    ULONG InSend = 0;

    for (int i=8; i<32;)
    {
        switch (*reinterpret_cast<PUCHAR>(Pos+i))
        {
            case 0x50:  // push eax
            case 0x51:  // push ecx
            case 0x52:  // push edx
            case 0x53:  // push ebx
            case 0x54:  // push esp
            case 0x55:  // push ebp
            case 0x56:  // push esi
            case 0x57:  // push edi
                i += 1;
                break;

            case 0x8B:  // mov esi, imm32
                i += 6;
                break;

            // VA_WS2SEND
            case 0xA1:  // mov  esi, imm32
                InSend = *reinterpret_cast<ULONG*>(Pos+i+1);
                i += 5;
                break;

            // VA_WS2RECV
            case 0xA3:  // mov [imm32], eax
                InRecv = *reinterpret_cast<ULONG*>(Pos+i+1);
                i += 5;
                break;

            case 0x75:  // jne  rel8
            case 0x85:  // test eax, eax
                break;

            default:
                i += 1;
                break;
        }

        if (InSend & InRecv)
            break;
    }

    g_Ctx.Addr[VA_WS2SEND] = InSend;
    g_Ctx.Addr[VA_WS2RECV] = InRecv;

    DbgPrintW(_L("VA_WS2SEND=%08X\n"), g_Ctx.Addr[VA_WS2SEND]);
    DbgPrintW(_L("VA_WS2RECV=%08X\n"), g_Ctx.Addr[VA_WS2RECV]);
}

//------------------------------------------------------------------------------
//  Function:   Investigator03
//  Notes   :   VA_SCENEBASE
//              VC_DRAWTEXT
//------------------------------------------------------------------------------
void Investigator03()
{
    DbgPrintW(_L("Investigator03 Tick=%08X\n"), GetTickCount());
/*
  // 2007-11-13aSakexe
  // 004CCE48
  8Drr********      lea     reg_1, [ebp-imm32]
  6A00              push    0
  6A00              push    0
  68FFFF9600        push    96FFFFh
  5r                push    reg_1
  6A01              push    1
  B9********        mov     ecx, imm32              ; VA_SCENEBASE
  E8********        call    rel32                   ; VC_DRAWTEXT

  // 004CD087
  B9********        mov     ecx, imm32              ; VA_SCENEBASE
  6A00              push    0
  6A00              push    0
  68FFFF9600        push    96FFFFh
  5r                push    reg_1
  6A01              push    1
  E8********        call    rel32                   ; VC_DRAWTEXT
*/
    ULONG Pos = 0;
    UCHAR Pattern[] = {
        0x68, 0xFF, 0xFF, 0x96, 0x00,
    };

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    ULONG SceneBase = 0;
    ULONG DrawText = 0;

    for (int i=-7; i<16;)
    {
        switch (*reinterpret_cast<PUCHAR>(Pos+i))
        {
            case 0x50:  // push eax
            case 0x51:  // push ecx
            case 0x52:  // push edx
            case 0x53:  // push ebx
            case 0x54:  // push esp
            case 0x55:  // push ebp
            case 0x56:  // push esi
            case 0x57:  // push edi
                i += 1;
                break;

            case 0x6A:  // push imm8
                i += 2;
                break;

            // VA_SCENEBASE
            case 0xB9:  // mov  ecx, imm32
                SceneBase = *reinterpret_cast<ULONG*>(Pos+i+1);
                i += 5;
                break;

            // VC_DRAWTEXT
            case 0xE8:  // call rel32
                if (ULONG Value = GetEIP(Pos+i))
                {
                    DrawText = Value;
                    i += 5;
                }
                break;

            default:
                i += 1;
                break;
        }

        if (SceneBase & DrawText)
            break;
    }

    g_Ctx.Addr[VA_SCENEBASE] = SceneBase;
    g_Ctx.Addr[VC_DRAWTEXT] = DrawText;

    DbgPrintW(_L("VA_SCENEBASE=%08X\n"), g_Ctx.Addr[VA_SCENEBASE]);
    DbgPrintW(_L("VC_DRAWTEXT=%08X\n"), g_Ctx.Addr[VC_DRAWTEXT]);
}

//------------------------------------------------------------------------------
//  Function:   Investigator04
//  Notes   :   VA_GAMEBASE
//------------------------------------------------------------------------------
void Investigator04()
{
    DbgPrintW(_L("Investigator04 Tick=%08X\n"), GetTickCount());
/*
  // 2007-11-13aSakexe
  // 0058B4D4
  68********        push    imm32
  6A00              push    0
  B9********        mov     ecx, imm32

  // 006730FF
  68********        push    imm32
  5r                push    reg_1
  B9********        mov     ecx, imm32
*/
    ULONG Pos = 0;

    if (FindStrFirstA("login.rsw", &Pos))
        DbgPrintW(_L("CodeString POS=%08X\n"), Pos);
    else
        return;

    UCHAR Pattern[] = {
        0x68, 0x00, 0x00, 0x00, 0x00,
    };
    RtlCopyMemory(Pattern+1, &Pos, sizeof(ULONG));

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    ULONG GameBase = 0;

    for (int i=0; i<16;)
    {
        switch (*reinterpret_cast<PUCHAR>(Pos+i))
        {
            case 0x50:  // push eax
            case 0x51:  // push ecx
            case 0x52:  // push edx
            case 0x53:  // push ebx
            case 0x54:  // push esp
            case 0x55:  // push ebp
            case 0x56:  // push esi
            case 0x57:  // push edi
                i += 1;
                break;

            case 0x6A:  // push imm8
                i += 2;
                break;

            // VA_GAMEBASE
            case 0xB9:  // mov  ecx, imm32
                GameBase = *reinterpret_cast<ULONG*>(Pos+i+1);
                i += 5;
                break;

            default:
                i += 1;
                break;
        }
    }

    g_Ctx.Addr[VA_GAMEBASE] = GameBase;

    DbgPrintW(_L("VA_GAMEBASE=%08X\n"), g_Ctx.Addr[VA_GAMEBASE]);
}

//------------------------------------------------------------------------------
//  Function:   Investigator05
//  Notes   :   VC_GETMSGSTR
//------------------------------------------------------------------------------
void Investigator05()
{
    DbgPrintW(_L("Investigator05 Tick=%08X\n"), GetTickCount());
/*
  // 2007-11-13aSakexe
  // 00486CBC
  6A**              push    imm8
  6A**              push    imm8
  6864FFFF00        push    0FFFF64h
  68********        push    imm32
  E8********        call    rel32
*/
    ULONG Pos = 0;
    UCHAR Pattern[] = {
        0x68, 0x64, 0xFF, 0xFF, 0x00,
    };

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    // VC_GETMSGPTR
    if (ULONG Value = GetEIP(Pos+10))
        g_Ctx.Addr[VC_GETMSGSTR] = Value;

    DbgPrintW(_L("VC_GETMSGSTR=%08X\n"), g_Ctx.Addr[VC_GETMSGSTR]);
}

//------------------------------------------------------------------------------
//  Function:   Investigator06
//  Notes   :   VA_CMDTABLE
//------------------------------------------------------------------------------
void Investigator06()
{
    DbgPrintW(_L("Investigator06 Tick=%08X\n"), GetTickCount());
/*
  // 2007-11-13aSakexe
  // 0057B696
  FF24rr********    jmp     [disp32+reg_1*4]
  8D450E            lea     eax, [ebp+0Eh]
  66C7450ED300      mov     word ptr [ebp+0Eh], 0D3h

  // 2008-03-28aRagexe
  // 0056D0A7
  FF24rr********    jmp     [disp32+reg_1*4]
  8D450E            lea     eax, [ebp+0Eh]
  66C7450EC200      mov     word ptr [ebp+0Eh], 0C2h
*/
    ULONG Pos = 0;
    UCHAR Pattern[] = {
        0x66, 0xC7, 0x45, 0x0E, 0xD3, 0x00
    };

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    // VA_CMDTABLE
    if (CompareOp(Pos-10, OP_FF24, 2))
    {
        ULONG Value = *reinterpret_cast<ULONG*>(Pos-7);
        g_Ctx.Addr[VA_CMDTABLE] = Value;
    }

    DbgPrintW(_L("VA_CMDTABLE=%08X\n"), g_Ctx.Addr[VA_CMDTABLE]);
}

//------------------------------------------------------------------------------
//  Function:   Investigator07
//  Notes   :   VA_ACTORBASE
//              VC_GETACTORCLASSINDEX
//------------------------------------------------------------------------------
void Investigator07()
{
    DbgPrintW(_L("Investigator07 Tick=%08X\n"), GetTickCount());
/*
  // 2007-11-13aSakexe
  // 00665E80
  B9********        mov     ecx, imm32              ; VA_ACTORBASE
  E8********        call    rel32                   ; VC_GETACTORCLASSINDEX
  3DCE0F0000        cmp     eax, 0FCEh
  7C**              jl      rel8
  B9********        mov     ecx, imm32              ; VA_ACTORBASE
  E8********        call    rel32                   ; VC_GETACTORCLASSINDEX
  3DD10F0000        cmp     eax, 0FD1h
  7F**              jg      rel8
  B8********        mov     eax, imm32              ; "effect\\i_p_SAINT.tga"
  5D                pop     ebp
  C20800            ret     8h
*/
    ULONG Pos = 0;

    if (FindStrFirstA("effect\\i_p_SAINT.tga", &Pos))
        DbgPrintW(_L("CodeString POS=%08X\n"), Pos);
    else
        return;

    UCHAR Pattern[] = {
        0xB8, 0x00, 0x00, 0x00, 0x00,
    };
    RtlCopyMemory(Pattern+1, &Pos, sizeof(ULONG));

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    // VA_ACTORBASE
    if (CompareOp(Pos-34, OP_B9, 1))
    {
        ULONG Value = *reinterpret_cast<ULONG*>(Pos-33);
        g_Ctx.Addr[VA_ACTORBASE] = Value;
    }

    // VC_GETACTORCLASSINDEX
    if (ULONG Value = GetEIP(Pos-29))
        g_Ctx.Addr[VC_GETOWNCLASSINDEX] = Value;

    DbgPrintW(_L("VA_ACTORBASE=%08X\n"), g_Ctx.Addr[VA_ACTORBASE]);
    DbgPrintW(_L("VC_GETOWNCLASSINDEX=%08X\n"), g_Ctx.Addr[VC_GETOWNCLASSINDEX]);
}

//------------------------------------------------------------------------------
//  Function:   Investigator08
//  Notes   :   VA_ACTORBASE
//              VA_ACTORCLASSNAMETABLE
//              VC_GETOWNCLASSINDEX
//------------------------------------------------------------------------------
void Investigator08()
{
    DbgPrintW(_L("Investigator08 Tick=%08X\n"), GetTickCount());
/*
  // 2007-11-13aSakexe
  // 005BB25A
  E8********            call    rel32                           ; VC_GETOWNCLASSINDEX
  85C0                  test    eax, eax
  75**                  jne     rel8
  A1********            mov     eax, moffs32                    ; VA_ACTORCLASSNAMETABLE
  C780****************  mov     dword ptr [eax+imm32], imm32    ; "TaeKwon Girl"
  EB**                  jmp     rel8
  8Brr********          mov     reg_1, m32
  C78r****************  mov     dword ptr [reg_1+imm32], imm32  ; "TaeKwon Boy"
  B9********            mov     ecx, imm32                      ; VA_ACTORBASE
  E8********            call    rel32                           ; VC_GETOWNCLASSINDEX
  
*/
    ULONG Pos = 0;

    if (FindStrFirstA("TaeKwon Girl", &Pos))
        DbgPrintW(_L("CodeString POS=%08X\n"), Pos);
    else
        return;

    UCHAR Pattern[] = {
        0x00, 0x00, 0x00, 0x00,
        0xEB,
    };
    RtlCopyMemory(Pattern, &Pos, sizeof(ULONG));

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    // VA_ACTORCLASSNAMETABLE
    // eaxÇ≈âºíË
    if (CompareOp(Pos-11, OP_A1, 1))
    {
        ULONG Value = *reinterpret_cast<ULONG*>(Pos-10);
        g_Ctx.Addr[VA_ACTORCLASSNAMETABLE] = Value;
    }

    // VA_ACTORBASE
    if (CompareOp(Pos+21, OP_B9, 1))
    {
        ULONG Value = *reinterpret_cast<ULONG*>(Pos+22);
        g_Ctx.Addr[VA_ACTORBASE] = Value;
    }

    // VC_GETOWNCLASSINDEX
    if (ULONG Value = GetEIP(Pos+26))
        g_Ctx.Addr[VC_GETOWNCLASSINDEX] = Value;

    DbgPrintW(_L("VA_ACTORCLASSNAMETABLE=%08X\n"), g_Ctx.Addr[VA_ACTORCLASSNAMETABLE]);
    DbgPrintW(_L("VA_ACTORBASE=%08X\n"), g_Ctx.Addr[VA_ACTORBASE]);
    DbgPrintW(_L("VC_GETOWNCLASSINDEX=%08X\n"), g_Ctx.Addr[VC_GETOWNCLASSINDEX]);
}

//------------------------------------------------------------------------------
//  Function:   Investigator09
//  Notes   :   VC_GETSKILLNAME
//------------------------------------------------------------------------------
void Investigator09()
{
    DbgPrintW(_L("Investigator09 Tick=%08X\n"), GetTickCount());
/*
  // 2007-11-13aSakexe
  // 004A2FE8
  8Brr**                mov     reg_1, [ebp-imm8]
  8D4D**                lea     ecx, [ebp-imm8]
  5r                    push    reg_1
  E8********            call    rel32               ; VC_GETSKILLNAME
  50                    push    eax
  8Brr********          mov     reg_2, [reg_3+imm32]
  8Drr********          lea     reg_4, [ebp-imm32]
  8Brrrr********        mov     reg_5, [disp32+reg_2*4]
  5r                    push    reg_5
  68********            push    imm32               ; "%s %s	(Sp : %d)"
  5r                    push    reg_4
  E8********            call    rel32
*/
    ULONG Pos = 0;

    if (FindStrFirstA("%s %s (Sp : %d)", &Pos))
        DbgPrintW(_L("CodeString POS=%08X\n"), Pos);
    else
        return;

    UCHAR Pattern[] = {
        0x68, 0x00, 0x00, 0x00, 0x00,
    };
    RtlCopyMemory(Pattern+1, &Pos, sizeof(ULONG));

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    // VC_GETSKILLNAME
    if (ULONG Value = GetEIP(Pos-26))
        g_Ctx.Addr[VC_GETSKILLNAME] = Value;

    DbgPrintW(_L("VC_GETSKILLNAME=%08X\n"), g_Ctx.Addr[VC_GETSKILLNAME]);
}

//------------------------------------------------------------------------------
//  Function:   Investigator10
//  Notes   :   VC_GETPACKETLENGTH
//------------------------------------------------------------------------------
void Investigator10()
{
    DbgPrintW(_L("Investigator10 Tick=%08X\n"), GetTickCount());
/*
  // 2007-11-13aSakexe
  // 0058B8A9
  E8********            call    rel32
  8BC8                  mov     ecx, eax
  E8********            call    rel32               ; VC_GETPACKETLENGTH
  50                    push    eax
  E8********            call    rel32
  8BC8                  mov     ecx, eax
  E8********            call    rel32
  8D45**                lea     eax, [ebp-imm8]
  8D8D********          lea     ecx, [ebp-imm32]
  50                    push    eax
  68********            push    imm32               ; "[ %s ]"
  51                    push    ecx
  E8********            call    rel32               ; _sprintf
*/
    ULONG Pos = 0;

    if (FindStrFirstA("[ %s ]", &Pos))
        DbgPrintW(_L("CodeString POS=%08X\n"), Pos);
    else
        return;

    UCHAR Pattern[] = {
        0x68, 0x00, 0x00, 0x00, 0x00,
    };
    RtlCopyMemory(Pattern+1, &Pos, sizeof(ULONG));

    if (FindBinFirst(Pattern, sizeof(Pattern), &Pos))
        DbgPrintW(_L("CodePattern POS=%08X\n"), Pos);
    else
        return;

    // VC_GETPACKETLENGTH
    if (ULONG Value = GetEIP(Pos-28))
        g_Ctx.Addr[VC_GETPACKETLENGTH] = Value;

    DbgPrintW(_L("VC_GETPACKETLENGTH=%08X\n"), g_Ctx.Addr[VC_GETPACKETLENGTH]);
}

//------------------------------------------------------------------------------
//  Function:   FindPacketLengthContainer
//  Notes   :   VA_PACKETLENGTHCONTAINER
//------------------------------------------------------------------------------
void FindPacketLengthContainer()
{
    DbgPrintW(_L("FindPacketLengthContainer Tick=%08X\n"), GetTickCount());

    ULONG Base = ClGetNetworkInstance();
    ULONG Pos = 0;
    ULONG Result = 0;

    for (ULONG i=Base; i<Base+PACKETLENGTHCONTAINER_SEARCH_RANGE; i+=4)
    {
        __try
        {
            ULONG Value = *reinterpret_cast<ULONG*>(i);
            PPACKETLENGTH_CONTAINER Entry = reinterpret_cast<PPACKETLENGTH_CONTAINER>(Value);

            if (Entry == NULL)
                continue;
            if (Entry->Color != 0)
                continue;
            if (Entry->Left)
            {
                if (Entry->Left->Color == 1)
                {
                    Pos = i;
                    Result = Value;
                }
            }
        }
        __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
        {
            continue;
        }

        if (Result)
            break;
    }

    g_Ctx.Addr[VA_PACKETLENGTHCONTAINER] = Result;

    DbgPrintW(_L("VA_PACKETLENGTHCONTAINER=%08X %08X %08X\n"), g_Ctx.Addr[VA_PACKETLENGTHCONTAINER], Pos, Pos-4);
}

//------------------------------------------------------------------------------
//  Function:   CMDIndexToCMDAddress
//  Notes   :   
//------------------------------------------------------------------------------
ULONG CMDIndexToCMDAddress(ULONG Index)
{
    ULONG Table = g_Ctx.Addr[VA_CMDTABLE];
    ULONG Result = 0;

    // Ç»Ç≠ÇƒÇ‡ë±çsÇ≥ÇπÇΩÇ¢ÇÃÇ≈ó·äOèàóùÇÇ≥ÇπÇÈ
    __try
    {
        ULONG Pos = *reinterpret_cast<ULONG*>(Table+(Index<<2));
        if (CompareOp(Pos, OP_8B, 1))
            Result = *reinterpret_cast<ULONG*>(Pos+2);
    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
    {
    }

    return Result;
}

//------------------------------------------------------------------------------
//  Function:   GetCMDAddress
//  Notes   :   
//------------------------------------------------------------------------------
void GetCMDAddress()
{
    DbgPrintW(_L("GetCMDAddress Tick=%08X\n"), GetTickCount());

    if (g_Ctx.Addr[VA_CMDTABLE] == 0)
        return;

    if (ULONG Addr = CMDIndexToCMDAddress(CMDOFS_BM))
    {
        g_Ctx.Addr[VA_STATE_BM] = Addr;
        g_Ctx.Addr[VA_STATE_BMINPUT] = Addr + 8;
    }
    if (ULONG Addr = CMDIndexToCMDAddress(CMDOFS_WI))
        g_Ctx.Addr[VA_STATE_WI] = Addr;
    if (ULONG Addr = CMDIndexToCMDAddress(CMDOFS_SH))
        g_Ctx.Addr[VA_STATE_SH] = Addr;

    DbgPrintW(_L("VA_STATE_SH=%08X\n"), g_Ctx.Addr[VA_STATE_SH]);
    DbgPrintW(_L("VA_STATE_WI=%08X\n"), g_Ctx.Addr[VA_STATE_WI]);
    DbgPrintW(_L("VA_STATE_BM=%08X\n"), g_Ctx.Addr[VA_STATE_BM]);
    DbgPrintW(_L("VA_STATE_BMINPUT=%08X\n"), g_Ctx.Addr[VA_STATE_BMINPUT]);
}

//------------------------------------------------------------------------------
//  Function:   RunInvestigator
//  Notes   :   
//------------------------------------------------------------------------------
void RunInvestigator()
{
    ULONG Threshold = g_Pref.Threshold;

    if (Threshold > 0)
    {
        PIMAGE_NT_HEADERS NTHdr = GetImageNTHeaders(GetModuleHandle(NULL));

        // Index=0ÇÕ.TEXTÇ…Ç»ÇÈÇÕÇ∏
        for (INT32 i=1; i<NTHdr->FileHeader.NumberOfSections; i++)
        {
            PIMAGE_SECTION_HEADER SectHdr = reinterpret_cast<PIMAGE_SECTION_HEADER>(NTHdr+1) + i;
            if (SectHdr->Misc.VirtualSize >= Threshold)
            {
                lcl_PEDataSection = i;
                break;
            }
        }
    }

    DbgPrintW(_L(".data THRESHOLD=%08X\n"), g_Pref.Threshold);
    DbgPrintW(_L("lcl_PEDataSeciont=%d\n"), lcl_PEDataSection);

    Investigator01();
    Investigator02();
    Investigator03();
    Investigator04();
    Investigator05();
    Investigator06();
    Investigator07();
    Investigator08();
    Investigator09();
    Investigator10();

    FindPacketLengthContainer();
    GetCMDAddress();
}
