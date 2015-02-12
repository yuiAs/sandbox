#include "PacketHandler.h"
#include "Module.h"
#include "Client.h"
#include "ROSaveChat.h"
#include <process.h>
#include "WinAdvApi.h"

//------------------------------------------------------------------------------
//  Function:   PacketHandlerThread
//  Note    :   
//------------------------------------------------------------------------------
unsigned int __stdcall PacketHandlerThread(PVOID Parameter)
{
    reinterpret_cast<PacketHandler*>(Parameter)->Worker();
    return 0;
}

//------------------------------------------------------------------------------
//  Function:   APCSaveChat
//  Note    :   
//------------------------------------------------------------------------------
void WINAPI APCSaveChat(ULONG_PTR Parameter)
{
    PCHATPACKET Item = reinterpret_cast<PCHATPACKET>(Parameter);
    reinterpret_cast<ROSaveChat*>(Item->This)->Save(Item);
    HeapFree(GetProcessHeap(), 0, reinterpret_cast<PVOID>(Parameter));
}

//------------------------------------------------------------------------------
//  Function:   MsgEnchantArm
//  Note    :   
//------------------------------------------------------------------------------
void MsgEnchantArm(USHORT Property, UCHAR Flag)
{
    if (Flag & 0x01)
    {
        USHORT StringID = MSG_NOMSG;

        switch (Property)
        {
            case EFST_PROPERTYFIRE:
                StringID = MSG_PROPERTY_FIRE;
                break;
            case EFST_PROPERTYWATER:
                StringID = MSG_PROPERTY_WATER;
                break;
            case EFST_PROPERTYWIND:
                StringID = MSG_PROPERTY_WIND;
                break;
            case EFST_PROPERTYGROUND:
                StringID = MSG_PROPERTY_GROUND;
                break;
            case EFST_PROPERTYDARK:
                StringID = MSG_PROPERTY_DARK;
                break;
            case EFST_PROPERTYTELEKINESIS:
                StringID = MSG_PROPERTY_TELEKINESIS;
                break;
        }

        ClDrawTextExA(COLOR_NOTICE1, MSG_ENCHANTARM, ClGetMsgStr(StringID));
    }
    else
    {
        PSTR String = ClGetMsgStr(MSG_RESTORE_PROPERTY_A);
        if (String)
            ClDrawTextA(String, COLOR_NOTICE2);
    }
}

//------------------------------------------------------------------------------
//  Function:   MsgChangeEffectState
//  Note    :   
//------------------------------------------------------------------------------
void MsgChangeEffectState(PSTR String, UCHAR Flag)
{
    if (Flag & 0x01)
        ClDrawTextExA(COLOR_NOTICE1, MSG_ADDSTATE, String);
    else
        ClDrawTextExA(COLOR_NOTICE2, MSG_REMOVESTATE, String);
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   PacketHandler::PacketHandler
//  Note    :   
//------------------------------------------------------------------------------
PacketHandler::PacketHandler()
        : m_Chat(NULL), m_Thread(NULL), m_Terminate(FALSE)
{
    m_Ctx.EnchantArm = NULL;
    Init();
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::~PacketHandler
//  Note    :   
//------------------------------------------------------------------------------
PacketHandler::~PacketHandler()
{
    Release();
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::Init
//  Note    :   
//------------------------------------------------------------------------------
void PacketHandler::Init()
{
    DbgPrintW(_L("PacketHandler::Init Tick=%08X\n"), GetTickCount());

    Release();
    // Allocate
    m_Ctx.EnchantArm = new EnchantMgr;

    // ROSaveChat
    if (g_Pref.AutoSaveChat)
        m_Chat = new ROSaveChat(g_Pref.WhisperType);

    ResetContext();
    m_Ctx.ObjectID = 0;

    // CreateThread
    if (m_Thread == NULL)
    {
        m_Terminate = FALSE;
        m_Thread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, PacketHandlerThread, this, 0, NULL));
    }

    DbgPrintW(_L("PacketHandlerThread HANDLE=%08X\n"), m_Thread);
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::Release
//  Note    :   
//------------------------------------------------------------------------------
void PacketHandler::Release()
{
    if (m_Thread)
    {
        Terminate();
        CloseHandle(m_Thread);
        m_Thread = 0;
    }
    if (m_Ctx.EnchantArm)
    {
        delete m_Ctx.EnchantArm;
        m_Ctx.EnchantArm = NULL;
    }
    if (m_Chat)
    {
        delete m_Chat;
        m_Chat = NULL;
    }
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::ResetContext
//  Note    :   
//------------------------------------------------------------------------------
void PacketHandler::ResetContext()
{
    m_Ctx.TrueSight = FALSE;
    m_Ctx.NoDivDmg = FALSE;
    m_Ctx.EnchantArm->Release(TRUE);
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::Suspend
//  Note    :   
//------------------------------------------------------------------------------
void PacketHandler::Suspend()
{
    SuspendThread(m_Thread);
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::Resume
//  Note    :   
//------------------------------------------------------------------------------
void PacketHandler::Resume()
{
    while (ResumeThread(m_Thread)>1);
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::Terminate
//  Note    :   
//------------------------------------------------------------------------------
void PacketHandler::Terminate()
{
    m_Terminate = TRUE;
    Resume();
    WaitForSingleObject(m_Thread, 1*1000);
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::Worker
//  Note    :   
//------------------------------------------------------------------------------
void PacketHandler::Worker()
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
    SuspendThread(GetCurrentThread());

    while (m_Terminate==FALSE)
    {
        ULONG Tick = GetTickCount();
        m_Ctx.EnchantArm->Check(Tick);
        SleepEx(50, TRUE);
    }
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::IsOwnOID
//  Note    :   
//------------------------------------------------------------------------------
BOOL PacketHandler::IsOwnOID(ULONG ObjectID)
{
    return (ObjectID==m_Ctx.ObjectID) ? TRUE : FALSE;
}

BOOL PacketHandler::IsOwnOID(PUCHAR Buffer)
{
    return IsOwnOID(*reinterpret_cast<ULONG*>(Buffer));
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::RecvParse
//  Note    :   
//------------------------------------------------------------------------------
int PacketHandler::RecvParse(PUCHAR Buffer, int Length)
{
    DbgPrintW(_L("RECV %08X LEN=%d OP=%04X"), GetTickCount(), Length, *reinterpret_cast<USHORT*>(Buffer));
#ifdef DBG
    if (PVOID Tmp = CreateHexDumpW(Buffer, Length, 0))
    {
        OutputDebugStringW(reinterpret_cast<PWSTR>(Tmp));
        ReleaseHexDump(Tmp);
    }
#endif

    switch (*reinterpret_cast<USHORT*>(Buffer))
    {
        // ACK
        case PACKET_AC_ACK_AUTH:
            AckAuth(Buffer);
            break;
        case PACKET_CC_ACK_CONNECT:
            AckConnectChar(Buffer);
            break;
        case PACKET_ZC_ACK_CONNECT:
            AckConnectZone(Buffer);
            break;
        case PACKET_ZC_ACK_REQ_SEED:
            AckGameGuard(Buffer);
            break;
        // NOR
        case PACKET_ZC_ERASE_ACTOR:
            OnEraseActor(Buffer);
            break;
        case PACKET_ZC_PORTAL:
        case PACKET_ZC_PORTAL_ZONE:
            OnMoveDistance(Buffer);
            break;
        case PACKET_ZC_ADD_TEMPORARY_SKILL:
            OnAddSkill(Buffer);
            break;
        case PACKET_ZC_EFFECT_STATE:
            OnSetObjectEffectState(Buffer);
            break;
        // LOG
        case PACKET_ZC_CHAT:
        case PACKET_ZC_CHAT_OWN:
        case PACKET_ZC_WHISPER:
        case PACKET_ZC_ACK_WHISPER:
        case PACKET_ZC_BROADCAST:
        case PACKET_ZC_CHAT_PARTY:
        case PACKET_ZC_MPV_CHARACTER:
        case PACKET_ZC_CHAT_GUILD:
        case PACKET_ZC_MULTICAST:
            OnChat(Buffer, Length);
            break;
        // PRIVATE
#ifdef PRIVATE_BUILD
//        case PACKET_ZC_ATTACK_DAMAGE:
//            OnAttackDamage(Buffer);
//            break;
        case PACKET_ZC_SKILL_DAMAGE:
            OnSkillDamage(Buffer);
            break;
        case PACKET_ZC_LOOK_OPTION:
            OnSetObjectLookOption(Buffer);
            break;
/*
        case 0x02EE:
            {
                if (*reinterpret_cast<USHORT*>(Buffer+58) != 0)
                    *reinterpret_cast<USHORT*>(Buffer+58) = 0x0000;
            }
            break;
*/
#endif
    }

    return Length;
}

//------------------------------------------------------------------------------
//  Function:   PacketHandler::SendParse
//  Note    :   
//------------------------------------------------------------------------------
int PacketHandler::SendParse(PUCHAR Buffer, int Length)
{
    DbgPrintW(_L("SEND %08X LEN=%d OP=%04X"), GetTickCount(), Length, *reinterpret_cast<USHORT*>(Buffer));
#ifdef DBG
    if (PVOID Tmp = CreateHexDumpW(Buffer, Length, 0))
    {
        OutputDebugStringW(reinterpret_cast<PWSTR>(Tmp));
        ReleaseHexDump(Tmp);
    }
#endif
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

// OP:  PACKET_AC_ACK_AUTH
void PacketHandler::AckAuth(PUCHAR Buffer)
{
    Suspend();

    ULONG ObjectID = *reinterpret_cast<ULONG*>(Buffer+8);
    m_Ctx.ObjectID = ObjectID;
    m_Ctx.EnchantArm->Init(ObjectID);

    DbgPrintW(_L("PACKET_AC_ACK_AUTH OID=%08X\n"), ObjectID);
}

// OP:  PACKET_CC_ACK_CONNECT
void PacketHandler::AckConnectChar(PUCHAR Buffer)
{
    Suspend();

    m_Ctx.EnchantArm->Release(TRUE);

    if (g_Pref.DefCmd[CMD_WI])
        ClSetCMDState(VA_STATE_WI, 1);
    if (g_Pref.DefCmd[CMD_SH])
        ClSetCMDState(VA_STATE_SH, 1);
}

// OP:  PACKET_ZC_ACK_CONNECT
void PacketHandler::AckConnectZone(PUCHAR Buffer)
{
#ifdef PRIVATE_BUILD
    // Load
    m_Ctx.TrueSight = GetPrivateProfileIntW(L"EXTENTION", L"DEFAULT_TRUESIGHT", 0, MOD_PREFERENCE);
    m_Ctx.NoDivDmg = GetPrivateProfileIntW(L"EXTENTION", L"DONOT_DIVIDE_DAMAGE", 0, MOD_PREFERENCE);

    if (m_Ctx.TrueSight)
        ClSetEffectState(m_Ctx.ObjectID, EFST_TRUESCENE, 1);
#endif
    // Zone間移動時にアイテム属性付与を発行
    m_Ctx.EnchantArm->Restore();

    Resume();

    DbgPrintW(_L("PACKET_ZC_ACK_CONNECT SvTick=%08X\n"), *reinterpret_cast<ULONG*>(Buffer+2));
}

// OP:  PACKET_GG_ACK_REQ_QUERY
void PacketHandler::AckGameGuard(PUCHAR Buffer)
{
    // 0x0259 <SVType>.B
    // 0x01: Account
    // 0x02: Char
    switch (*reinterpret_cast<UCHAR*>(Buffer+2))
    {
        case 0x01:      // Into Account
            ResetContext();
            m_Ctx.ObjectID = 0;
            break;
        case 0x02:      // Into Char
            ResetContext();
            break;
        default:
            __assume(0);
    }
}

// OP:  PACKET_ZC_ERASE_ACTOR
void PacketHandler::OnEraseActor(PUCHAR Buffer)
{
    if (IsOwnOID(Buffer+2))
    {
        // DEAD
        if (*reinterpret_cast<UCHAR*>(Buffer+6) == 0x01)
            m_Ctx.EnchantArm->Release(FALSE);
    }
}

// OP:  PACKET_ZC_PORTAL
//      PACKET_ZC_PORTAL_ZONE
void PacketHandler::OnMoveDistance(PUCHAR Buffer)
{
    if (g_Pref.BMFix == FALSE)
        return;

    // BMが一時的に解除されるバグを抑制
    if (ClCheckCMDState(VA_STATE_BM, 1))
        ClSetCMDState(VA_STATE_BMINPUT, 0);
}

// OP:  PACKET_ZC_ADD_TEMPORARY_SKILL
void PacketHandler::OnAddSkill(PUCHAR Buffer)
{
    USHORT SkillID = *reinterpret_cast<USHORT*>(Buffer+2);
    USHORT SkillLv = *reinterpret_cast<USHORT*>(Buffer+8);

    if (SkillID != ITEM_ENCHANTARMS)
        return; // アイテム属性付与のみ対応

    USHORT Type = EFST_NONE;

    switch (SkillLv)
    {
        case 0x0002:
            Type = EFST_PROPERTYWATER;
            break;
        case 0x0003:
            Type = EFST_PROPERTYGROUND;
            break;
        case 0x0004:
            Type = EFST_PROPERTYFIRE;
            break;
        case 0x0005:
            Type = EFST_PROPERTYWIND;
            break;
        case 0x0008:
            Type = EFST_PROPERTYDARK;
            break;
    }

    if (Type != EFST_NONE)
        m_Ctx.EnchantArm->Enchant(Type, GetTickCount());
}

// OP:  PACKET_ZC_EFFECT_STATE
void PacketHandler::OnSetObjectEffectState(PUCHAR Buffer)
{
    if (IsOwnOID(Buffer+4) == FALSE)
        return;

    USHORT Type = *reinterpret_cast<USHORT*>(Buffer+2);
    UCHAR Flag = *reinterpret_cast<UCHAR*>(Buffer+8);

    switch (Type)
    {
        case EFST_BREAKWEAPON:
            m_Ctx.EnchantArm->Release(FALSE);
            break;

#ifdef PRIVATE_BUILD
        case EFST_HALLUCINATION:
            if (Flag)
            {
                *reinterpret_cast<UCHAR*>(Buffer+8) &= 0;
                ClDrawTextA(MSG_ST_HALLUCINATION, COLOR_NOTICE2);
            }
            break;
#endif

        case EFST_PROPERTYFIRE:
        case EFST_PROPERTYWATER:
        case EFST_PROPERTYWIND:
        case EFST_PROPERTYGROUND:
        case EFST_PROPERTYDARK:
        case EFST_PROPERTYTELEKINESIS:
            if (Flag)
                m_Ctx.EnchantArm->Release(FALSE);   // アイテム属性付与がされていたら解除させる
            MsgEnchantArm(Type, Flag);
            break;

        case EFST_EXPLOSIONSPIRITS:
            MsgChangeEffectState(ClGetSkillName(MO_EXPLOSIONSPIRITS), Flag);
            break;

        case EFST_BERSERK:
            MsgChangeEffectState(ClGetSkillName(LK_BERSERK), Flag);
            break;

        case EFST_ASSUMPTIO:
            MsgChangeEffectState(ClGetSkillName(HP_ASSUMPTIO), Flag);
            break;

        case EFST_NIGHT:
            {
                // 拳聖の場合は奇跡
                ULONG Class = ClGetOwnClassIndex();
                if ((Class==ACT_STARGRADIATOR_M) || (Class==ACT_STARGRADIATOR_F))
                    MsgChangeEffectState(SKNAME_MIRACLE, Flag);
            }
            break;

#ifdef PRIVATE_BUILD
        case EFST_RG_CCONFINE_M:
        case EFST_RG_CCONFINE_S:
            MsgChangeEffectState(ClGetSkillName(RG_CLOSECONFINE), Flag);
            break;
#endif
    }
}

// OP:  ry
void PacketHandler::OnChat(PUCHAR Buffer, int Length)
{
    if (g_Pref.AutoSaveChat == FALSE)
        return;

    SIZE_T AllocLen = Length + sizeof(CHATPACKET);
    PVOID AllocBuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, AllocLen);
    if (AllocBuf != NULL)
    {
        PCHATPACKET Item = reinterpret_cast<PCHATPACKET>(AllocBuf);
        Item->This = reinterpret_cast<PVOID>(m_Chat);
        Item->ChatType = 0;
        GetLocalTimeAsFileTime(&Item->RecvTm);
        RtlCopyMemory(Item->Data, Buffer, Length);
        QueueUserAPC(APCSaveChat, m_Thread, reinterpret_cast<ULONG_PTR>(Item));
    }
}

void PacketHandler::OnAttackDamage(PUCHAR Buffer)
{
#ifdef PRIVATE_BUILD
#endif
}

void PacketHandler::OnSkillDamage(PUCHAR Buffer)
{
#ifdef PRIVATE_BUILD
    if (m_Ctx.NoDivDmg == FALSE)
        return;

    if (IsOwnOID(Buffer+8))
    {
        if (*reinterpret_cast<UCHAR*>(Buffer+32) == 8)
        {
            *reinterpret_cast<USHORT*>(Buffer+30) = 1;  // 単発は1で固定
            *reinterpret_cast<UCHAR*>(Buffer+32) = 6;   // 単発に変更
        }
    }
#endif
}

void PacketHandler::OnSetObjectLookOption(PUCHAR Buffer)
{
#ifdef PRIVATE_BUILD
    if (IsOwnOID(Buffer+2))
    {
        // BODY
        USHORT Param1 = *reinterpret_cast<USHORT*>(Buffer+6);
        // HEALTH
        USHORT Param2 = *reinterpret_cast<USHORT*>(Buffer+8);
        // EFLO
        //ULONG Param3 = *reinterpret_cast<ULONG*>(Buffer+10);

        switch (Param1)
        {
            case BODY_FREEZING:
                ClDrawTextA(MSG_ST_FREEZING, COLOR_NOTICE2);
                break;
            case BODY_STUN:
                ClDrawTextA(MSG_ST_STUN, COLOR_NOTICE2);
                break;
            case BODY_SLEEP:
                ClDrawTextA(MSG_ST_SLEEP, COLOR_NOTICE2);
                break;
        }

        if (Param2 & HEALTH_BLIND)
        {
            *reinterpret_cast<USHORT*>(Buffer+8) ^= HEALTH_BLIND;
            ClDrawTextA(MSG_ST_BLIND, COLOR_NOTICE2);
        }
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Define
//------------------------------------------------------------------------------
#ifndef ENCHANTARM_COUNT
#define ENCHANTARM_COUNT    180000
#endif

//------------------------------------------------------------------------------
//  Class   :   EnchantMgr
//  Note    :
//------------------------------------------------------------------------------
EnchantMgr::EnchantMgr()
    : m_ObjectID(0), m_Count(0), m_Property(EFST_NONE)
{
}

EnchantMgr::~EnchantMgr()
{
}

void EnchantMgr::Init(ULONG ObjectID)
{
    m_ObjectID = ObjectID;
}

void EnchantMgr::Enchant(USHORT Property, ULONG Count)
{
    Release(FALSE);

    if (Property != EFST_NONE)
    {
        MsgEnchantArm(Property, 1);
        ClSetEffectState(m_ObjectID, Property, 1);
        m_Property = Property;
        m_Count = Count + ENCHANTARM_COUNT;
    }
}

void EnchantMgr::Restore()
{
    if (IsEnchant())
        Enchant(m_Property, m_Count-ENCHANTARM_COUNT);
}

void EnchantMgr::Release(BOOL OnReset)
{
    if ((OnReset==FALSE) && (m_Property!=EFST_NONE))
    {
        MsgEnchantArm(m_Property, 0);
        ClSetEffectState(m_ObjectID, m_Property, 0);
    }

    m_Property = EFST_NONE;
    m_Count = 0;
}

BOOL EnchantMgr::IsEnchant()
{
    return (m_Property!=EFST_NONE) ? TRUE : FALSE;
}

void EnchantMgr::Check(ULONG Tick)
{
    if (m_Count < Tick)
        Release(FALSE);
}
