#ifndef RPACKET_H
#define RPACKET_H

#include "win32.h"
#include "client.h"		// for PLT_ENTRY
#include "rParser.h"

/////////////////////////////////////////////////////////////////////////////////


class ROPacket
{
private:

	ROPacket(const ROPacket& _class);
	ROPacket& operator=(const ROPacket& _class);

private:

	PUCHAR m_Garbage;
	ULONG m_GarbageLength;

	ULONG m_PLT;

	ROParser* m_Parser;

public:

	ROPacket();
	virtual ~ROPacket();

public:

	void Init(ULONG PLT);

public:

	int GetLength(PUCHAR Buffer);

	int Recv(char* Buffer, int Length);
	int Send(char* Buffer, int Length);

private:

	void Release();

	int GetLength(PTR_PLT_ENTRY Entry, ULONG CurrentOp);
};


#endif	// #ifndef RPACKET_H
