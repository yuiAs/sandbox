//! @file   packethandler.h
#pragma once


class PacketHandler
{
    // Structure
public:
    typedef struct _SESSION_CONTEXT
    {
        ULONG CtrlFlag;
        ULONG CharOID;
        USHORT EnchantArm;
    } SESSION_CONTEXT, *PSESSION_CONTEXT;

    // Constructer/Destructer
public:
    PacketHandler();
    virtual ~PacketHandler();

    // Methods
public:
    int Recv(PUCHAR Buffer, int Length);

private:
    void AckSelectChar(PUCHAR Buffer, int Length);
    void AckConnectZone(PUCHAR Buffer, int Length);
    void OnMoveDistance(PUCHAR Buffer, int Length);
    void OnAddSkillTemporay(PUCHAR Buffer, int Length);
    void OnSetObjectEffectState(PUCHAR Buffer, int Length);
    void OnDamageBySkill(PUCHAR Buffer, int Length);
    void OnSetObjectLookOption(PUCHAR Buffer, int Length);
    void OnChat(PUCHAR Buffer, int Length);
    void FixPacket(PUCHAR Buffer, int Length);

    void ResetContext();
    void EnchantArm(USHORT Type, UCHAR Flag);

    // Data members
private:
    SESSION_CONTEXT session_;

};
