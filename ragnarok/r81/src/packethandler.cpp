//! @file   packethandler.cpp

#include "stdafx.h"
#include "module.h"
#include "client.h"
#include "packethandler.h"


//! Constructer
PacketHandler::PacketHandler()
{
    ResetContext();
}

//! Destructer
PacketHandler::~PacketHandler()
{
}

//! Reset context
void PacketHandler::ResetContext()
{
    RtlZeroMemory(&session_, sizeof(SESSION_CONTEXT));
    session_.EnchantArm = 0x0043;
}

//! Parse recv buffer
int PacketHandler::Recv(PUCHAR Buffer, int Length)
{
    DbgPrintW(L"[R] %08X %04X LEN=%d\n", GetTickCount(), *reinterpret_cast<USHORT*>(Buffer), Length);
    DbgDump(Buffer, Length, 0);

    switch (*reinterpret_cast<USHORT*>(Buffer))
    {
        // PACKET_CC_ACK_SELECT_CHARACTER
        case 0x0071:
            AckSelectChar(Buffer, Length);
            break;
        // PACKET_ZC_CHAT
        case 0x008D:
        // PACKET_ZC_CHAT_SELF
        case 0x008E:
        // PACKET_ZC_WHISPER
        case 0x0097:
        // PACKET_ZC_ACK_WHISPER
        case 0x0098:
        // PACKET_ZC_BROADCAST
        case 0x009A:
        // PACKET_ZC_CHAT_PARTY
        case 0x0109:
        // PACKET_ZC_MPV_CHARACTER
        case 0x010C:
        // PACKET_ZC_CHAT_GUILD
        case 0x017F:
        // PACKET_ZC_MULTICAST
        case 0x01C3:
//        // PACKET_ZC_MSG
//        case 0x02C1: OnChat(Buffer, Length);                    break;
//        // PACKET_ZC_MSG_VARS
//        case 0x02C2: OnChat(Buffer, Length);                    break;
             OnChat(Buffer, Length);
            break;
        // PACKET_ZC_PORTAL
        case 0x0091:
        // PACKET_ZC_PORTAL_ZONE
        case 0x0092:
            OnMoveDistance(Buffer, Length);
            break;
        // PACKET_ZC_ADD_TEMPORARY_SKILL
        case 0x0147:
            OnAddSkillTemporay(Buffer, Length);
            break;
        // PACKET_ZC_EFFECT_STATE
        case 0x0196:
            OnSetObjectEffectState(Buffer, Length);
            break;
        // PACKET_ZC_SKILL_DAMAGE
        case 0x01DE:
            OnDamageBySkill(Buffer, Length);
            break;
        // PACKET_ZC_LOOK_OPTION
        case 0x0229:
            OnSetObjectLookOption(Buffer, Length);
            break;
        // PACKET_ZC_ACK_CONNECT
        case 0x02EB:
            FixPacket(Buffer, Length);
            AckConnectZone(Buffer, Length);
            break;
        // PACKET_ZC_CREATE_ACTOR
        case 0x02EE:
            FixPacket(Buffer, Length);
            break;
    }

    return Length;
}

//! PACKET_CC_ACK_SELECT_CHARACTER
void PacketHandler::AckSelectChar(PUCHAR Buffer, int Length)
{
    session_.CharOID = *reinterpret_cast<ULONG*>(Buffer+2);

    DbgPrintW(L"ObjectID A=%08X C=%08X\n", Client::GetOwnOID(), session_.CharOID);
}

//! PACKET_ZC_ACK_CONNECT
void PacketHandler::AckConnectZone(PUCHAR Buffer, int Length)
{
    ULONG mask = CTRL_FIX_BM|CTRL_AUTOSAVECHAT|CTRL_TRUESIGHT|CTRL_DONOT_DIVIDE_DAMAGE;
    session_.CtrlFlag = Context.CtrlFlag & mask;

    if (session_.CtrlFlag & CTRL_TRUESIGHT)
    {
        if (Client::SetEffectStateSelf(0x00B8,1))   // EFST_TRUESCENE
            __noop;
    }
    if (session_.EnchantArm != 0x0043)
        EnchantArm(session_.EnchantArm, 1);
}

//! PACKET_ZC_PORTAL
//! PACKET_ZC_PORTAL_ZONE
void PacketHandler::OnMoveDistance(PUCHAR Buffer, int Length)
{
    if (session_.CtrlFlag & CTRL_FIX_BM)
        __noop;
}

//! PACKET_ZC_ADD_TEMPORARY_SKILL
void PacketHandler::OnAddSkillTemporay(PUCHAR Buffer, int Length)
{
    USHORT skillID = *reinterpret_cast<USHORT*>(Buffer+2);
    USHORT skillLv = *reinterpret_cast<USHORT*>(Buffer+8);

    if (skillID != 0x01EC)
        return;

    USHORT type = 0x0043;

    switch (skillLv)
    {
        case 0x0002: type = 0x005B; break;  // EFST_PROPERTYWATER
        case 0x0003: type = 0x005D; break;  // EFST_PROPERTYGROUND
        case 0x0004: type = 0x005A; break;  // EFST_PROPERTYFIRE
        case 0x0005: type = 0x005C; break;  // EFST_PROPERTYWIND
        case 0x0008: type = 0x0092; break;  // EFST_PROPERTYDARK
    }

    if (type != 0x0043)
        EnchantArm(type, 1);
}

//! PACKET_ZC_EFFECT_STATE
void PacketHandler::OnSetObjectEffectState(PUCHAR Buffer, int Length)
{
    if (Client::IsOwnOID(Buffer+4) == false)
        return;

    USHORT state = *reinterpret_cast<USHORT*>(Buffer+2);
    UCHAR flag = *reinterpret_cast<UCHAR*>(Buffer+8);

    switch (state)
    {
        case 0x0021:    // EFST_BREAKWEAPON
            if (session_.EnchantArm != 0x0043)
                EnchantArm(session_.EnchantArm, 0);
            break;
        case 0x0022:    // EFST_HALLCINATION
            *reinterpret_cast<UCHAR*>(Buffer+8) = 0;
            break;
        case 0x005A:    // EFST_PROPERTYFIRE
        case 0x005B:    // EFST_PROPERTYWATER
        case 0x005C:    // EFST_PROPERTYWIND
        case 0x005D:    // EFST_PROPERTYGROUND
        case 0x0092:    // EFST_PROPERTYDARK
        case 0x0094:    // EFST_PROPERTYTELEKINESIS
            if (session_.EnchantArm != 0x0043)
                EnchantArm(session_.EnchantArm, 0);
            EnchantArm(state, 1);
            break;
        case 0x0056:    // EFST_EXPLOSIONSPIRITS
            break;
        case 0x006B:    // EFST_BERSERK
            break;
        case 0x006E:    // EFST_ASSUMPTIO
            break;
        case 0x00A0:    // EFST_NIGHT
            {
                ULONG index = Client::GetClassIndexSelf();
                if ((index==0x0FCF) || (index==0x0FD0))
                    __noop;
            }
            break;
        case 0x00C8:    // EFST_RG_CCONFINE_M
        case 0x00C9:    // EFST_RG_CCONFINE_S
            break;

    }
}

//! PACKET_ZC_SKILL_DAMAGE
void PacketHandler::OnDamageBySkill(PUCHAR Buffer, int Length)
{
    if (session_.CtrlFlag & CTRL_DONOT_DIVIDE_DAMAGE)
    {
        if (Client::IsOwnOID(Buffer+8))
        {
            if (*reinterpret_cast<UCHAR*>(Buffer+32) == 8)
            {
                *reinterpret_cast<USHORT*>(Buffer+30) = 1;
                *reinterpret_cast<UCHAR*>(Buffer+32) = 6;
            }
        }
    }
}

//! PACKET_ZC_LOOK_OPTION
void PacketHandler::OnSetObjectLookOption(PUCHAR Buffer, int Length)
{
    if (Client::IsOwnOID(Buffer+2))
    {
        USHORT param1 = *reinterpret_cast<USHORT*>(Buffer+6);   // body
        USHORT param2 = *reinterpret_cast<USHORT*>(Buffer+8);   // health
//        ULONG param3 = *reinterpret_cast<ULONG*>(Buffer+10);    // extent

        char* msg = NULL;

        switch (param1)
        {
            case 0x0002: msg = NULL; break; // FREEZING
            case 0x0003: msg = NULL; break; // STUN
            case 0x0004: msg = NULL; break; // SLEEP
        }

        if (msg)
            __noop;

        if (param2 & 0x0010)    // BLIND
        {
            *reinterpret_cast<USHORT*>(Buffer+8) ^= 0x0010;
        }
    }
}

//! CHAT
void PacketHandler::OnChat(PUCHAR Buffer, int Length)
{
    if (session_.CtrlFlag & CTRL_AUTOSAVECHAT)
    {
        SIZE_T reqLen = Length + sizeof(CHATPACKET_LIST);

        PVOID buf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, reqLen);
        if (buf != NULL)
        {
            PCHATPACKET_LIST item = reinterpret_cast<PCHATPACKET_LIST>(buf);
            item->Ptr = this;
            item->Type = 0;
            GetLocalTimeAsFileTime(&item->Time);
            RtlCopyMemory(item->Data, Buffer, Length);

            QueueUserAPC(APCChatLog, GetCoreAPCThread(), reinterpret_cast<ULONG_PTR>(buf));
        }
    }
}

//! -
void PacketHandler::FixPacket(PUCHAR Buffer, int Length)
{
    switch (*reinterpret_cast<USHORT*>(Buffer))
    {
        case 0x02EB:
            {
/*
  2008/09/10
                00000183	23.49547577	[3676] [R] 0111201A 02EB LEN=13
                00000184	23.49553680	[3676]  eb 02 92 91 e4 07 0a 89  80 05 05 00 00            .............
*/
                USHORT codepage = *reinterpret_cast<USHORT*>(Buffer+11);
                if (codepage)
                    *reinterpret_cast<USHORT*>(Buffer+11) = 0;//932;
            }
            break;
        case 0x02EE:
            {
/*
  2008/09/10
                00000609	24.04589462	[3676] [R] 0111223D 02EE LEN=60
                00000610	24.04596901	[3676]  ee 02 71 a8 13 00 96 00  00 00 00 00 00 00 00 00   ..q.............
                00000611	24.04596901	[3676]  b1 0f 0c 00 c7 04 36 08  00 00 ec 00 95 01 01 00   ......6.........
                00000612	24.04596901	[3676]  00 00 00 00 5d 30 00 00  56 00 00 00 00 00 00 00   ....]0..V.......
                00000613	24.04596901	[3676]  00 00 09 c9 e4 05 05 00  63 00 00 00               ........c...
*/
                USHORT codepage = *reinterpret_cast<USHORT*>(Buffer+58);
                if (codepage)
                    *reinterpret_cast<USHORT*>(Buffer+58) = 0;//932;
            }
            break;
    }
}

//! -
void PacketHandler::EnchantArm(USHORT Type, UCHAR Flag)
{
    Client::MsgEnchangArm(Type, Flag);
    Client::SetEffectStateSelf(Type, Flag);

    session_.EnchantArm = (Flag&0x01) ? Type : 0x0043;
}
