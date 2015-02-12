//! @file   intelligence.cpp

#include "stdafx.h"
#include "module.h"
#include "client.h"

#if !defined(INTL_PACKETLENGTH_SEARCH_DEPTH)
#define INTL_PACKETLENGTH_SEARCH_DEPTH  0x030000
#endif


//! Instructions
typedef enum _INSTRUCTION86
{
    // cmp
    INS_39, INS_3D, INS_813D,
    // test
    INS_85,
    // mov
    INS_8A, INS_8A88, INS_8B, INS_8B0D, INS_8B15, INS_8B49, INS_8B88, INS_A1, INS_B9, INS_C7, INC_C745,
    // call
    INS_E8,
    // jmp
    INS_E9, INS_FF, INS_FF24,
    // jcc
    INS_0F84,
    INSTRUCTION86_MAX,
} INSTRUCTION86;

//! Opecode Table
#include <pshpack1.h>
UCHAR tblInstruction[][INSTRUCTION86_MAX] = {
    { 0x39, },                          // cmp  r/m32, r32
    { 0x3D, /*id*/ },                   // cmp  eax, imm32
    { 0x81, 0x3D, /*id*/ },             // cmp  r/m32, imm32
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

//! Variables
ULONG DataSection = 2;

//! -
PIMAGE_NT_HEADERS GetPENTHeaders(HANDLE Module)
{
    PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(Module);
    return reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<LONG>(dos)+dos->e_lfanew);
}

//! -
void GetPESectionSize(ULONG Offset, PULONG Base, PULONG Size)
{
    PIMAGE_NT_HEADERS nt = GetPENTHeaders(GetModuleHandle(NULL));
    PIMAGE_SECTION_HEADER section = reinterpret_cast<PIMAGE_SECTION_HEADER>(nt+1) + Offset;

    if (Base)
        *Base = nt->OptionalHeader.ImageBase + section->VirtualAddress;
    if (Size)
        *Size = section->Misc.VirtualSize;
}

//! -
bool CompareBin(const PVOID Binary, SIZE_T Length, LONG Index, ULONG Base, ULONG Size, PULONG Result)
{
    *Result = 0;

    if (Index != -1)
        GetPESectionSize(Index, &Base, &Size);

    __try
    {
        for (ULONG i=Base; i<(Base+Size)-Length; i++)
        {
            if (RtlEqualMemory(reinterpret_cast<PVOID>(i), Binary, Length))
            {
                *Result = i;
                break;
            }
        }
    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
    {
        DbgPrintW(L"*** Except ACCESS_VIOLATION CompareBin\n");
        *Result = 0;
    }

    return (*Result!=0);
}

//! -
inline bool FindBinary(const PUCHAR Binary, SIZE_T BinaryLength, PULONG Result)
{
    return CompareBin(Binary, BinaryLength, 0, 0, 0, Result);
}

//! -
inline bool FindStringA(const PSTR String, PULONG Result)
{
    return CompareBin(String, strlen(String)+1, DataSection, 0, 0, Result);
}

//! -
inline bool CompareInstructionSet(ULONG Address, INSTRUCTION86 Index, SIZE_T Length)
{
    return (RtlEqualMemory(tblInstruction[Index], reinterpret_cast<PVOID>(Address), Length)==TRUE);
}

//! -
bool CalcEIP(ULONG Address, PULONG Result)
{
    if (CompareInstructionSet(Address, INS_E8, 1))
    {
        ULONG relative = *reinterpret_cast<ULONG*>(Address+1);
        *Result = Address + relative + 5;
        return true;
    }
    return false;
}


//! ADDR_HULLCINATION
void AutoCorrect_01()
{
/*
  ; 2008-08-27aSakexe
  ; 00410EC8
  mov       eax, [007C1494]                     ; ADDR_HULLCINATION
  test      eax, eax
  jz        0041119A
  ; Ç±ÇÍà»ç~Ç™ HULLCINATION ÇÃèàóùÇ»ÇÃÇ≈ jmp Ç…Ç∑ÇÈÇ© [007C1494] ÇèÌÇ…0Ç…Ç∑ÇÈÇ©ÇÃÇ«ÇøÇÁÇ©

  ; 00567345
  A1********        mov     eax, moffs32        ; ADDR_HULLCINATION
  85C0              test    eax, eax
  74**              je      rel8
  85DB              test    ebx, ebx
  7C**              jl      rel8
  81FB60EA0000      cmp     ebx, 0EA60h
  7E08              jle     rel8
*/
    ULONG offset = 0;
    UCHAR pattern[] = {
        0x81, 0xFB, 0x60, 0xEA, 0x00, 0x00,
    };

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    if (CompareInstructionSet(offset+13, INS_A1, 1))
    {
        ULONG result = *reinterpret_cast<ULONG*>(offset+12);
        Context.Addr[ADDR_HULLCINATION] = result;
    }

    DbgPrintW(L"ADDR_HULLCINATION=%08X\n", Context.Addr[ADDR_HULLCINATION]);
}

//! ADDR_BATTLEMODE
//! AOFS_JCC_DOUBLELETTER
//! ADDR_BMINPUT
void AutoCorrect_02()
{
/*
  ; 2008-08-27aSakexe
  ; 0069494B
  ; ebx = lParam (ÇÃâ∫à 1byte)
  ; edi = lParam
  ; esi = wParam
  A1********        mov     eax, moffs32            ; ADDR_BATTLEMODE
  33DB              xor     ebx, ebx
  3BC3              cmp     eax, ebx
  74**		        jz      rel8
  833D********0D    cmp     r/m32, 0Dh              ; 0Dh = VK_RETURN
  75**              jnz     rel8                    ; AOFS_JCC_DOUBLELETTER
  ; Ç±Ç±Ç≈ jmp Ç∑ÇÈÇ∆ïsñ°Ç¢
  391D********      cmp     r/m32, ebx              ; ADDR_BMINPUT
  74**              jz      rel8
*/
}

//! AEIP_GETPACKETINSTANCE
//! ADDR_ZONEPROCEDURE
void AutoCorrect_03()
{
/*
  ; 2007-11-13aSakexe
  ; 0058DEA2
  8Drr********      lea     reg_1, [ebp-imm32]
  5r                push    reg_1
  E8********        call    rel32                   ; AEIP_GETPACKETINSTANCE
  ; Packet::GetInstance()Ç›ÇΩÇ¢Ç»ÅB
  8BC8              mov     ecx, eax
  E8********        call    rel32
  0FBFC0            mov     eax, ax
  83C08D            add     eax, 0FFFFFF8Dh         ; -73h
  3D********        cmp     eax, imm32
  0F87********      ja      rel32
  FF2485********    jmp     [disp32+eax*4]
*/
    ULONG offset = 0;
    UCHAR pattern[] = {
        0x0F, 0xBF, 0xC0,
        0x83, 0xC0, 0x8D,
    };

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    // ADDR_ZONEPROCEDURE
    if (CompareInstructionSet(offset+17, INS_FF24, 2))
    {
        ULONG result = *reinterpret_cast<ULONG*>(offset+20);
        Context.Addr[ADDR_ZONEPROCEDURE] = result;
    }

    // AEIP_GETPACKETINSTANCE
    ULONG result = 0;
    if (CalcEIP(offset-12, &result))
        Context.Addr[AEIP_GETPACKETINSTANCE] = result;

    DbgPrintW(L"ADDR_ZONEPROCEDURE=%08X\n", Context.Addr[ADDR_ZONEPROCEDURE]);
    DbgPrintW(L"AEIP_GETPACKETINSTANCE=%08X\n", Context.Addr[AEIP_GETPACKETINSTANCE]);
}

//! ADDR_WS2RECV
//! ADDR_WS2SEND
void AutoCorrect_04()
{
    ULONG offset = 0;

    if (FindStringA("recv", &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    UCHAR pattern[] = {
        0x68, 0x00, 0x00, 0x00, 0x00,
    };
    *reinterpret_cast<ULONG*>(pattern+1) = offset;

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    ULONG recv = 0;
    ULONG send = 0;

    for (int i=8; i<32;)
    {
        switch (*reinterpret_cast<UCHAR*>(offset+i))
        {
            case 0x50:                      // push eax
            case 0x51:                      // push ecx
            case 0x52:                      // push edx
            case 0x53:                      // push ebx
            case 0x54:                      // push esp
            case 0x55:                      // push ebp
            case 0x56:                      // push edi
            case 0x57:  i += 1; break;      // push esi
            case 0x8B:  i += 6; break;      // mov  r32, r/m32
            case 0xA1:  send = *reinterpret_cast<ULONG*>(offset+i+1);
                        i += 5; break;      // mov  eax, moffs32
            case 0xA3:  recv = *reinterpret_cast<ULONG*>(offset+i+1);
                        i += 5; break;      // mov  moffs32, eax
//            case 0x75:                      // jne  rel8
//            case 0x85:  break;              // test eax, eax
            default:    i += 1; break;
        }

        if (send & recv)
            break;
    }

    Context.Addr[ADDR_WS2SEND] = send;
    Context.Addr[ADDR_WS2RECV] = recv;

    DbgPrintW(L"ADDR_WS2SEND=%08X\n", Context.Addr[ADDR_WS2SEND]);
    DbgPrintW(L"ADDR_WS2RECV=%08X\n", Context.Addr[ADDR_WS2RECV]);
}

//! ADDR_SCENE
//! AEIP_DRAWSTRING
void AutoCorrect_05()
{
/*
  ; 2007-11-13aSakexe
  ; 004CCE48
  8Drr********      lea     reg_1, [ebp-imm32]
  6A00              push    0
  6A00              push    0
  68FFFF9600        push    96FFFFh
  5r                push    reg_1
  6A01              push    1
  B9********        mov     ecx, imm32              ; ADDR_SCENE
  E8********        call    rel32                   ; AEIP_DRAWSTRING

  ; 004CD087
  B9********        mov     ecx, imm32              ; ADDR_SCENE
  6A00              push    0
  6A00              push    0
  68FFFF9600        push    96FFFFh
  5r                push    reg_1
  6A01              push    1
  E8********        call    rel32                   ; AEIP_DRAWSTRING
*/
    ULONG offset = 0;
    UCHAR pattern[] = {
        0x68, 0xFF, 0xFF, 0x96, 0x00,
    };

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    ULONG sceneBase = 0;
    ULONG drawString = 0;

    for (int i=-7; i<16;)
    {
        switch (*reinterpret_cast<UCHAR*>(offset+i))
        {
            case 0x50:                      // push eax
            case 0x51:                      // push ecx
            case 0x52:                      // push edx
            case 0x53:                      // push ebx
            case 0x54:                      // push esp
            case 0x55:                      // push ebp
            case 0x56:                      // push edi
            case 0x57:  i += 1; break;      // push esi
            case 0x6A:  i += 2; break;      // push imm8
            case 0xB9:  sceneBase = *reinterpret_cast<ULONG*>(offset+i+1);
                        i += 5; break;      // mov  ecx, imm32
            case 0xE8:  CalcEIP(offset+i, &drawString) ? i+=5 : i+=1; break;    // call rel32
            default:    i += 1; break;
        }

        if (sceneBase & drawString)
            break;
    }

    Context.Addr[ADDR_SCENE] = sceneBase;
    Context.Addr[AEIP_DRAWSTRING] = drawString;

    DbgPrintW(L"ADDR_SCENE=%08X\n", Context.Addr[ADDR_SCENE]);
    DbgPrintW(L"AEIP_DRAWSTRING=%08X\n", Context.Addr[AEIP_DRAWSTRING]);
}

//! ADDR_GAME
void AutoCorrect_06()
{
/*
  ; 2007-11-13aSakexe
  ; 0058B4D4
  68********        push    imm32                   ; "login.rsw"
  6A00              push    0
  B9********        mov     ecx, imm32              ; ADDR_GAME

  ; 006730FF
  68********        push    imm32                   ; "login.rsw"
  5r                push    reg_1
  B9********        mov     ecx, imm32              ; ADDR_GAME
*/
    ULONG offset = 0;

    if (FindStringA("login.rsw", &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    UCHAR pattern[] = {
        0x68, 0x00, 0x00, 0x00, 0x00,
    };
    *reinterpret_cast<ULONG*>(pattern+1) = offset;

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    ULONG gameBase = 0;

    for (int i=0; i<16;)
    {
        switch (*reinterpret_cast<UCHAR*>(offset+i))
        {
            case 0x50:                      // push eax
            case 0x51:                      // push ecx
            case 0x52:                      // push edx
            case 0x53:                      // push ebx
            case 0x54:                      // push esp
            case 0x55:                      // push ebp
            case 0x56:                      // push edi
            case 0x57:  i += 1; break;      // push esi
            case 0x6A:  i += 2; break;      // push imm8
            case 0xB9:  gameBase = *reinterpret_cast<ULONG*>(offset+i+1);
                        i += 5; break;      // mov  ecx, imm32
            default:    i += 1; break;
        }

        if (gameBase)
            break;
    }

    Context.Addr[ADDR_GAME] = gameBase;

    DbgPrintW(L"ADDR_SCENE=%08X\n", Context.Addr[ADDR_SCENE]);
}

//! AEIP_GETMESSAGE
void AutoCorrect_07()
{
/*
  ; 2007-11-13aSakexe
  ; 00486CBC
  6A**              push    imm8
  6A**              push    imm8
  6864FFFF00        push    0FFFF64h
  68********        push    imm32
  E8********        call    rel32                   ; AEIP_GETMESSAGE
*/
    ULONG offset = 0;
    UCHAR pattern[] = {
        0x68, 0x64, 0xFF, 0xFF, 0x00,
    };

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    // AEIP_GETMESSAGE
    ULONG eip = 0;
    if (CalcEIP(offset+10, &eip))
        Context.Addr[AEIP_GETMESSAGE] = eip;

    DbgPrintW(L"AEIP_GETMESSAGE=%08X\n", Context.Addr[AEIP_GETMESSAGE]);
}

//! ADDR_ACTOR
//! AEIP_GETCLASSINDEXSELF
void AutoCorrect_08()
{
/*
  ; 2007-11-13aSakexe
  ; 00665E80
  B9********        mov     ecx, imm32              ; ADDR_ACTORBASE
  E8********        call    rel32                   ; AEIP_GETCLASSINDEXSELF
  3DCE0F0000        cmp     eax, 0FCEh
  7C**              jl      rel8
  B9********        mov     ecx, imm32              ; ADDR_ACTORBASE
  E8********        call    rel32                   ; AEIP_GETCLASSINDEXSELF
  3DD10F0000        cmp     eax, 0FD1h
  7F**              jg      rel8
  B8********        mov     eax, imm32              ; "effect\\i_p_SAINT.tga"
  5D                pop     ebp
  C20800            ret     8h
*/
    ULONG offset = 0;

    if (FindStringA("effect\\i_p_SAINT.tga", &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    UCHAR pattern[] = {
        0xB8, 0x00, 0x00, 0x00, 0x00,
    };
    *reinterpret_cast<ULONG*>(pattern+1) = offset;

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    // ADDR_ACTOR
    if (CompareInstructionSet(offset-34, INS_B9, 1))
    {
        ULONG result = *reinterpret_cast<ULONG*>(offset-33);
        Context.Addr[ADDR_ACTOR] = result;
    }

    // AEIP_GETCLASSINDEXSELF
    ULONG result = 0;
    if (CalcEIP(offset-29, &result))
        Context.Addr[AEIP_GETCLASSINDEXSELF] = result;

    DbgPrintW(L"ADDR_ACTOR=%08X\n", Context.Addr[ADDR_ACTOR]);
    DbgPrintW(L"AEIP_GETCLASSINDEXSELF=%08X\n", Context.Addr[AEIP_GETCLASSINDEXSELF]);
}

//! ADDR_ACTOR
//! ADDR_ACTORCLASSNAMETABLE
//! AEIP_GETCLASSINDEXSELF
void AutoCorrect_09()
{
/*
  ; 2007-11-13aSakexe
  ; 005BB25A
  E8********            call    rel32                           ; AEIP_GETCLASSINDEXSELF
  85C0                  test    eax, eax
  75**                  jne     rel8
  A1********            mov     eax, moffs32                    ; ADDR_CLASSNAMETABLE
  C780****************  mov     dword ptr [eax+imm32], imm32    ; "TaeKwon Girl"
  EB**                  jmp     rel8
  8Brr********          mov     reg_1, m32
  C78r****************  mov     dword ptr [reg_1+imm32], imm32  ; "TaeKwon Boy"
  B9********            mov     ecx, imm32                      ; ADDR_ACTOR
  E8********            call    rel32                           ; AEIP_GETCLASSINDEXSELF
  
*/
    ULONG offset = 0;

    if (FindStringA("TaeKwon Girl", &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    UCHAR pattern[] = {
        0x00, 0x00, 0x00, 0x00,
        0xEB,
    };
    *reinterpret_cast<ULONG*>(pattern) = offset;

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;
/*
    // ADDR_LASSNAMETABLE
    if (CompareInstructionSet(offset-11, INS_A1, 1))
    {
        ULONG result = *reinterpret_cast<ULONG*>(offset-10);
        Context.Addr[ADDR_ACTORCLASSNAMETABLE] = result;
    }
*/
    // ADDR_ACTOR
    if (CompareInstructionSet(offset+21, INS_B9, 1))
    {
        ULONG result = *reinterpret_cast<ULONG*>(offset+22);
        Context.Addr[ADDR_ACTOR] = result;
    }

    // AEIP_GETCLASSINDEXSELF
    ULONG result = 0;
    if (CalcEIP(offset+26, &result))
        Context.Addr[AEIP_GETCLASSINDEXSELF] = result;

//    DbgPrintW(L"ADDR_ACTORCLASSNAMETABLE=%08X *\n", Context.Addr[ADDR_ACTORCLASSNAMETABLE]);
    DbgPrintW(L"ADDR_ACTOR=%08X\n", Context.Addr[ADDR_ACTOR]);
    DbgPrintW(L"AEIP_GETCLASSINDEXSELF=%08X\n", Context.Addr[AEIP_GETCLASSINDEXSELF]);
}

//! AEIP_GETSKILLNAME
void AutoCorrect_0A()
{
/*
  ; 2007-11-13aSakexe
  ; 004A2FE8
  8Brr**                mov     reg_1, [ebp-imm8]
  8D4D**                lea     ecx, [ebp-imm8]
  5r                    push    reg_1
  E8********            call    rel32               ; AEIP_GETSKILLNAME
  50                    push    eax
  8Brr********          mov     reg_2, [reg_3+imm32]
  8Drr********          lea     reg_4, [ebp-imm32]
  8Brrrr********        mov     reg_5, [disp32+reg_2*4]
  5r                    push    reg_5
  68********            push    imm32               ; "%s %s	(Sp : %d)"
  5r                    push    reg_4
  E8********            call    rel32
*/
    ULONG offset = 0;

    if (FindStringA("%s %s (Sp : %d)", &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    UCHAR pattern[] = {
        0x68, 0x00, 0x00, 0x00, 0x00,
    };
    *reinterpret_cast<ULONG*>(pattern+1) = offset;

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    // AEIP_GETSKILLNAME
    ULONG result = 0;
    if (CalcEIP(offset-26, &result))
        Context.Addr[AEIP_GETSKILLNAME] = result;

    DbgPrintW(L"AEIP_GETSKILLNAME=%08X\n", Context.Addr[AEIP_GETSKILLNAME]);
}

//! ADDR_OWNAID
void AutoCorrect_0B()
{
/*
  ; 2008-11-13aSakexe.exe
  ; 005736DC
  68********            push    imm32               ; "_poison.wav"
  E8*******             call    rel32               ; 7à¯êîÇÃä÷êîÇæÇØÇ«äÑà§
  8Brr********          mov     reg_1, [reg_2+imm32]
  A1********            mov     eax, moffs32        ; ADDR_OWNAID

  ; 005A55EF
  813D********B9860100  cmp     r/m32, 000186B9h    ; ADDR_OWNAID
  0F85********          jne     rel32
*/
    ULONG offset = 0;
    UCHAR pattern[] = {
        0xB9, 0x86, 0x01, 0x00,
        0x0F, 0x85,
    };

    if (FindBinary(pattern, sizeof(pattern), &offset))
        DbgPrintW(L"FIND %08X\n", offset);
    else
        return;

    // ADDR_OWNAID
    if (CompareInstructionSet(offset-6, INS_813D, 2))
    {
         ULONG result = *reinterpret_cast<ULONG*>(offset-4);
        Context.Addr[ADDR_OWNAID] = result;
    }

    DbgPrintW(L"ADDR_OWNAID=%08X\n", Context.Addr[ADDR_OWNAID]);
}

//! ADDR_PACKETLENGTH
void AutoCorrect_PacketLength()
{
    ULONG base = reinterpret_cast<ULONG>(Client::GetPacketInstance());
    ULONG offset = 0;
    ULONG result = 0;

    for (ULONG i=base; i<base+INTL_PACKETLENGTH_SEARCH_DEPTH; i+=4)
    {
        __try
        {
            ULONG addr = *reinterpret_cast<ULONG*>(i);
            if (addr == 0)
                continue;

            Client::NODE_PACKETLENGTH* node = reinterpret_cast<Client::NODE_PACKETLENGTH*>(addr);
            if ((node->_Color==0) && node->_Left)
            {
                if (node->_Left->_Color == 1)
                {
                    offset = i;
                    result = addr;
                }
            }
        }
        __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
        {
            continue;
        }

        if (result) break;
    }

    Context.Addr[ADDR_PACKETLENGTH] = result;

    DbgPrintW(L"ADDR_PACKETLENGTH=%08X (%08X)\n", Context.Addr[ADDR_PACKETLENGTH], offset);
}

//! Detect .data section.
ULONG DetectDataSection(ULONG Threshold)
{
    ULONG result = DataSection;

    if (Threshold > 0)
    {
        PIMAGE_NT_HEADERS nt = GetPENTHeaders(GetModuleHandle(NULL));

        for (int i=1; i<nt->FileHeader.NumberOfSections; i++)
        {
            PIMAGE_SECTION_HEADER section = reinterpret_cast<PIMAGE_SECTION_HEADER>(nt+1) + i;
            if (section->Misc.VirtualSize >= Threshold)
            {
                result = i;
                break;
            }
        }
    }

    return result;
}

//! Startup
void AutoCorrect()
{
    DbgPrintW(L".data detecting threshold %08X\n", Pref.Threshold);
    DataSection = DetectDataSection(Pref.Threshold);
    DbgPrintW(L".data section index %d\n", DataSection);

//    DbgPrintW(L"AC01 TICK=%08X\n", GetTickCount());
//    AutoCorrect_01();
//    DbgPrintW(L"AC02 TICK=%08X\n", GetTickCount());
//    AutoCorrect_02();
    DbgPrintW(L"AC03 TICK=%08X\n", GetTickCount());
    AutoCorrect_03();
    DbgPrintW(L"AC04 TICK=%08X\n", GetTickCount());
    AutoCorrect_04();
    DbgPrintW(L"AC05 TICK=%08X\n", GetTickCount());
    AutoCorrect_05();
    DbgPrintW(L"AC06 TICK=%08X\n", GetTickCount());
    AutoCorrect_06();
    DbgPrintW(L"AC07 TICK=%08X\n", GetTickCount());
    AutoCorrect_07();
    DbgPrintW(L"AC08 TICK=%08X\n", GetTickCount());
    AutoCorrect_08();
    DbgPrintW(L"AC09 TICK=%08X\n", GetTickCount());
    AutoCorrect_09();
    DbgPrintW(L"AC0A TICK=%08X\n", GetTickCount());
    AutoCorrect_0A();
    DbgPrintW(L"AC0B TICK=%08X\n", GetTickCount());
    AutoCorrect_0B();
    DbgPrintW(L"AC90 TICK=%08X\n", GetTickCount());
    AutoCorrect_PacketLength();
}
