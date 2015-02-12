#ifndef _CLIENT_
#define _CLIENT_

#include <windows.h>
#include "EnumClient.h"
#include "EnumAegis.h"

//------------------------------------------------------------------------------
// Struct
//------------------------------------------------------------------------------
#include <pshpack1.h>
typedef struct _PACKETLENGTH_CONTAINER
{
    struct _PACKETLENGTH_CONTAINER* Left;
    struct _PACKETLENGTH_CONTAINER* Parent;
    struct _PACKETLENGTH_CONTAINER* Right;
    ULONG Op;
    ULONG Length;
    ULONG Color;
} PACKETLENGTH_CONTAINER, *PPACKETLENGTH_CONTAINER;

typedef struct _CLSKILLINFO_CONTAINER
{
    ULONG _Unk1;
    ULONG SkillID;
    ULONG _Unk3;
    ULONG _Unk4;
    ULONG CostSP;
    ULONG _Unk6;
    ULONG _Unk7;
    PSTR SkillIDString;
} CLSKILLINFO_CONTAINER, *PCLSKILLINFO_CONTAINER;
#include <poppack.h>

//------------------------------------------------------------------------------
// ClientApi.cpp
//------------------------------------------------------------------------------
PSTR ClGetMsgStr(ULONG MsgID);
PSTR ClGetSkillName(ULONG SkillID);
PSTR ClGetActorClassName(ULONG Index);
ULONG ClGetOwnClassIndex();
void ClDrawTextA(PSTR String, ULONG Color);
void ClDrawTextW(PWSTR String, ULONG Color);
void ClDrawTextExA(ULONG Color, PSTR Format, ...);
void ClDrawTextExW(ULONG Color, PWSTR Format, ...);
ULONG ClGetNetworkInstance();
int ClGetPacketLength(PVOID Buffer);
void ClCallZoneProcedure(USHORT Op, PVOID Buffer);
void ClSetEffectState(ULONG ObjectID, USHORT StateType, UCHAR Flag);
void ClSetCMDState(ULONG Index, LONG Value);
LONG ClGetCMDState(ULONG Index);
BOOL ClCheckCMDState(ULONG Index, LONG Value);

//------------------------------------------------------------------------------
// ClientInvestigator.cpp
//------------------------------------------------------------------------------
void RunInvestigator();


#endif  // _CLIENT_
