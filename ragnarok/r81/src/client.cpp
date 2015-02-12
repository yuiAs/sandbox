//! @file   client.cpp

#include "stdafx.h"
#include "client.h"
#include "module.h"


namespace Client
{

//! 引数の ObjectID が自分自身の ObjectID と一致するかどうか。
bool IsOwnOID(ULONG oid)
{
    return (GetOwnOID()==oid);
}

//! 引数の ObjectID が自分自身の ObjectID と一致するかどうか。
bool IsOwnOID(PUCHAR Buffer)
{
//    return (RtlEqualMemory(Buffer, reinterpret_cast<PVOID>(Context.Addr[ADDR_OWNAID]), sizeof(ULONG))==TRUE);
    return (GetOwnOID()==(*reinterpret_cast<ULONG*>(Buffer)));
}

//! 自分自身の ObjectID を返す。
__forceinline ULONG GetOwnOID()
{
    return (*reinterpret_cast<ULONG*>(Context.Addr[ADDR_OWNAID]));
}

//! MsgID に対応する msgstringtable.txt の文字列を取得する。
char* GetMessage(ULONG MsgID)
{
    ULONG function = Context.Addr[AEIP_GETMESSAGE];

    __asm
    {
        push    MsgID
        call    dword ptr [function]
        add     esp, 4
    }
}

//! Get own class index.
ULONG GetClassIndexSelf()
{
    ULONG function = Context.Addr[AEIP_GETCLASSINDEXSELF];
    ULONG ptrClass = Context.Addr[ADDR_ACTOR];

    __asm
    {
        mov     ecx, ptrClass
        call    dword ptr [function]
    }
}

//! SkillID のクライアント内部定義文字列を返す。
const char* GetSkillName(ULONG SkillID)
{
    SKILLINFO skillInfo;
    RtlZeroMemory(&skillInfo, sizeof(SKILLINFO));
    skillInfo._reserved1 = 1;
    skillInfo.SkillID = SkillID;

    ULONG function = Context.Addr[AEIP_GETSKILLNAME];

    __asm
    {
        lea     ecx, skillInfo
        call    dword ptr [function]
    }

    // 返値の eax は SKILLINFO.strSkillID と同値なのでそのまま通す。
    //return si.strSkillID;
}

//! Get Packet-class instance.
PVOID GetPacketInstance()
{
    ULONG function = Context.Addr[AEIP_GETPACKETINSTANCE];

    __asm
    {
        call    dword ptr [function]
    }
}

//! Get packet length.
int GetPacketLength(PVOID Buffer)
{
    USHORT op = *reinterpret_cast<USHORT*>(Buffer);
    NODE_PACKETLENGTH* node = reinterpret_cast<NODE_PACKETLENGTH*>(Context.Addr[ADDR_PACKETLENGTH])->_Parent;

    while ((node!=NULL) && (node->Op!=op))
        node = (op<node->Op) ? node->_Left : node->_Right;

    if (node && (node->Length!=0))
        return node->Length;
    else
        return 2;
}

////////////////////////////////////////////////////////////////////////////////

//! -
void DrawStringInternal(const char* String, ULONG Color, ULONG Attribute)
{
    ULONG function = Context.Addr[AEIP_DRAWSTRING];
    ULONG ptrClass = Context.Addr[ADDR_SCENE];

    __asm
    {
        push    0
        push    0
        push    Color
        push    String
        push    Attribute
        mov     ecx, ptrClass
        call    dword ptr [function]
    }
}

//! -
void DrawStringA(const char* String, ULONG Color)
{
    DrawStringInternal(String, Color, 1);
}

//! -
void DrawStringExA(ULONG Color, const char* FormatString, ...)
{
    char buf[CLIENT_DRAWSTRING_LIMIT];
    RtlZeroMemory(buf, sizeof(buf));

    va_list varList;
    va_start(varList, FormatString);

    int result = _vsnprintf(buf, CLIENT_DRAWSTRING_LIMIT-1, FormatString, varList);
    if (result != -1)
        DrawStringInternal(buf, Color, 1);

    va_end(varList);
}

//! -
void DrawStringW(const wchar_t* String, ULONG Color)
{
    char buf[CLIENT_DRAWSTRING_LIMIT];
    int result = WideCharToMultiByte(/*CP_ACP*/932, 0, String, -1, buf, CLIENT_DRAWSTRING_LIMIT, NULL, NULL);
    if (result != 0)
        DrawStringInternal(buf, Color, 1);
}

//! -
void DrawStringExW(ULONG Color, const wchar_t* FormatString, ...)
{
    wchar_t buf[CLIENT_DRAWSTRING_LIMIT];
    RtlZeroMemory(buf, sizeof(buf));

    va_list varList;
    va_start(varList, FormatString);

    int result = _vsnwprintf(buf, CLIENT_DRAWSTRING_LIMIT-1, FormatString, varList);
    if (result != -1)
        DrawStringW(buf, Color);

    va_end(varList);
}

////////////////////////////////////////////////////////////////////////////////

//! -
ULONG GetZoneProcedure(USHORT op)
{
    ULONG table = Context.Addr[ADDR_ZONEPROCEDURE];

    __asm
    {
        mov     esi, table
        movsx   eax, word ptr [op]
        add     eax, 0FFFFFF8Dh
        mov     edi, [esi+eax*4]
        xor     eax, eax
        mov     al, byte ptr [edi+9]
        cmp     eax, 0E8h
        mov     eax, 0
        jne     __DO_NOT_MATCH
        mov     edx, [edi+0Ah]
        lea     eax, [edi+edx+0Eh]
    __DO_NOT_MATCH:
    }
}

//! -
bool CallZoneProcedure(PVOID Buffer)
{
    bool result = false;
    USHORT op = *reinterpret_cast<USHORT*>(Buffer);

    ULONG function = GetZoneProcedure(op);
    if (function == 0)
    {
        DbgPrintW(L"*** Failure CallZoneProcedure OP=%04X\n", op);
        return result;
    }
    ULONG ptrClass = Context.Addr[ADDR_GAME];

    __try
    {
        __asm
        {
            mov     eax, ptrClass
            mov     ecx, [eax+4]
            push    Buffer
            call    dword ptr [function]
        }

        result = true;
    }
    __except (DbgExceptionInfo(GetExceptionInformation()))
    {
        DbgPrintW(L"*** Failure CallZoneProcedure OP=%04X PROCEDURE=%08X\n", op, function);
        result = false;
    }

    return result;
}

//! -
bool SetEffectState(ULONG oid, USHORT State, UCHAR Flag)
{
    UCHAR buf[9];
    *reinterpret_cast<USHORT*>(buf) = 0x0196;   // PACKET_ZC_EFFECT_STATE
    *reinterpret_cast<USHORT*>(buf+2) = State;
    *reinterpret_cast<ULONG*>(buf+4) = oid;
    *reinterpret_cast<UCHAR*>(buf+8) = Flag;

    return CallZoneProcedure(buf);
}

//! -
bool SetEffectStateSelf(USHORT State, UCHAR Flag)
{
    return SetEffectState(GetOwnOID(), State, Flag);
}

////////////////////////////////////////////////////////////////////////////////

//! -
void MsgEnchangArm(USHORT Type, UCHAR Flag)
{
    if (Flag & 0x01)
    {
        ULONG msgID = 0;

        switch (Type)
        {
            case 0x005A: msgID = 417; break;    // EFST_PROPERTYFIRE
            case 0x005B: msgID = 415; break;    // EFST_PROPERTYWATER
            case 0x005C: msgID = 418; break;    // EFST_PROPERTYWIND
            case 0x005D: msgID = 416; break;    // EFST_PROPERTYGROUND
            case 0x0092: msgID = 421; break;    // EFST_PROPERTYDARK
            case 0x0094: msgID = 422; break;    // EFST_PROPERTYTELEKINESIS
        }

        if (msgID != 0)
        {
            char* msg = GetMessage(msgID);
            if (msg)
                __noop;
        }
    }
    else
    {
        char* msg = GetMessage(471);    // MSG_RESTORE_PROPERTY_ARM
        if (msg)
            __noop;
    }
}

//! -
void MsgChangeEffectState(char* State, UCHAR Flag)
{
    if (Flag & 0x01)
        __noop;
    else
        __noop;
}


};
