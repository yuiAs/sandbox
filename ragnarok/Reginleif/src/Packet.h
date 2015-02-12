#ifndef _PACKET_
#define _PACKET_

#include <windows.h>
#include "WinAdvApi.h"
#include "PacketHandler.h"

//------------------------------------------------------------------------------
// Macro
//------------------------------------------------------------------------------
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
    TypeName(const TypeName&);              \
    void operator=(const TypeName&);
#endif

//------------------------------------------------------------------------------
// Define
//------------------------------------------------------------------------------
#ifndef SEED1_LENGTH
#define SEED1_LENGTH    16
#endif

//------------------------------------------------------------------------------
//  Class   :   Packet
//  Note    :
//------------------------------------------------------------------------------
class Packet : public PacketHandler
{
    typedef struct _LINKCONTAINER
    {
        LIST_ENTRY ListEntry;
        ULONG Length;
        UCHAR Data[1];
    } LINKCONTAINER, *PLINKCONTAINER;

private:
    ULONG m_PacketLength;

    BOOL m_SkipOID;
    BOOL m_CryptRound1;
    BOOL m_CryptRound2;
    UCHAR m_Seed1[SEED1_LENGTH];

    HANDLE m_Heap;
    LIST_ENTRY m_Container;
    ULONG m_ContainerLength;

public:
    Packet();
    virtual ~Packet();

private:
    DISALLOW_COPY_AND_ASSIGN(Packet);

public:
    void Init();
private:
    void Release();

public:
    int Recv(char* Buffer, int Length);
    int Send(char* Buffer, int Length);

private:
    int PreRecvParse(PUCHAR Buffer, int Length);
    int PreSendParse(PUCHAR Buffer, int Length);
    int GetLength(PUCHAR Buffer);

    void AllocEntry(PUCHAR Buffer, int Length);
};

#endif  // _PACKET_
