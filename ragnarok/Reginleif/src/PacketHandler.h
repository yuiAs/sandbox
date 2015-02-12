#ifndef _PACKETHANDLER_
#define _PACKETHANDLER_

#include <windows.h>

//------------------------------------------------------------------------------
// Macro
//------------------------------------------------------------------------------
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
    TypeName(const TypeName&);              \
    void operator=(const TypeName&);
#endif

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------
class EnchantMgr;
class ROSaveChat;

//------------------------------------------------------------------------------
// Struct
//------------------------------------------------------------------------------
typedef struct _SESSION_CONTEXT
{
    ULONG ObjectID;
    BOOL TrueSight;
    BOOL NoDivDmg;
    EnchantMgr* EnchantArm;
} SESSION_CONTEXT, *PSESSION_CONTEXT;

//------------------------------------------------------------------------------
//  Class   :   PacketHandler
//  Note    :
//------------------------------------------------------------------------------
class PacketHandler
{
protected:
    SESSION_CONTEXT m_Ctx;
    ROSaveChat* m_Chat;

protected:
    HANDLE m_Thread;
    BOOL m_Terminate;

public:
    PacketHandler();
    virtual ~PacketHandler();

private:
    DISALLOW_COPY_AND_ASSIGN(PacketHandler);

protected:
    virtual void Init();
    virtual void Release();

private:
    void ResetContext();

    void Suspend();
    void Resume();
    void Terminate();

public:
    virtual void Worker();

    virtual int RecvParse(PUCHAR Buffer, int Length);
    virtual int SendParse(PUCHAR Buffer, int Length);

protected:
    BOOL IsOwnOID(ULONG ObjectID);
    BOOL IsOwnOID(PUCHAR Buffer);

private:
    void AckAuth(PUCHAR Buffer);
    void AckConnectChar(PUCHAR Buffer);
    void AckConnectZone(PUCHAR Buffer);
    void AckGameGuard(PUCHAR Buffer);
    void OnEraseActor(PUCHAR Buffer);
    void OnMoveDistance(PUCHAR Buffer);
    void OnAddSkill(PUCHAR Buffer);
    void OnSetObjectEffectState(PUCHAR Buffer);
    void OnChat(PUCHAR Buffer, int Length);
    void OnAttackDamage(PUCHAR Buffer);
    void OnSkillDamage(PUCHAR Buffer);
    void OnSetObjectLookOption(PUCHAR Buffer);
};

//------------------------------------------------------------------------------
//  Class   :   EnchantMgr
//  Note    :
//------------------------------------------------------------------------------
class EnchantMgr
{
private:
    ULONG m_ObjectID;
    ULONG m_Count;
    USHORT m_Property;

public:
    EnchantMgr();
    virtual ~EnchantMgr();

public:
    void Init(ULONG ObjectID);
    void Enchant(USHORT Property, ULONG Count);
    void Restore();
    void Release(BOOL OnReset);
    BOOL IsEnchant();
    void Check(ULONG Tick);
};

#endif  // _PACKETHANDLER_
