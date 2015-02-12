//! @file   client.h
#pragma once

#if !defined(CLIENT_DRAWSTRING_LIMIT)
#define CLIENT_DRAWSTRING_LIMIT 104
#endif


namespace Client
{
    //! PacketLength table
    //! @see Red-black tree
    typedef struct _NODE_PACKETLENGTH
    {
        struct _NODE_PACKETLENGTH* _Left;
        struct _NODE_PACKETLENGTH* _Parent;
        struct _NODE_PACKETLENGTH* _Right;
        ULONG Op;
        ULONG Length;
        ULONG _Color;
    } NODE_PACKETLENGTH, *PNODE_PACKETLENGTH;

    //! SkillInfo table
    typedef struct _SKILLINFO
    {
        ULONG _reserved1;
        ULONG SkillID;
        ULONG _reserved3;
        ULONG _reserved4;
        ULONG CostSP;
        ULONG _reserved6;
        ULONG _reserved7;
        char* strSkillID;
    } SKILLINFO, *PSKILLINFO;

    //! client.cpp

    bool IsOwnOID(ULONG oid);
    bool IsOwnOID(PUCHAR Buffer);

    ULONG GetOwnOID();
    char* GetMessage(ULONG MsgID);
    ULONG GetClassIndexSelf();
    const char* GetSkillName(ULONG SkillID);
    PVOID GetPacketInstance();
    int GetPacketLength(PVOID Buffer);

    void DrawStringA(const char* String, ULONG Color);
    void DrawStringW(const wchar_t* String, ULONG Color);
    void DrawStringExA(ULONG Color, const char* FormatString, ...);
    void DrawStringExW(ULONG Color, const wchar_t* FormatString, ...);

    bool CallZoneProcedure(PVOID Buffer);
    bool SetEffectState(ULONG oid, USHORT State, UCHAR Flag);
    bool SetEffectStateSelf(USHORT State, UCHAR Flag);

    void MsgEnchangArm(USHORT Type, UCHAR Flag);
    void MsgChangeEffectState(char* State, UCHAR Flag);


};
