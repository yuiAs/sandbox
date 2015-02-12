#include "rPacket.h"
#include "intrnl.h"
#include "debug.h"

////////////////////////////////////////////////////////////////////////////////


ROPacket::ROPacket() :
	m_Garbage(NULL), m_GarbageLength(0), m_PLT(0), m_Parser(NULL)
{
}

ROPacket::~ROPacket()
{
	Release();
}

////////////////////////////////////////////////////////////////////////////////


void ROPacket::Init(ULONG PLT)
{
	if (m_Garbage == NULL)
		m_Garbage = reinterpret_cast<PUCHAR>(VirtualAlloc(NULL, 2048<<4, MEM_COMMIT, PAGE_READWRITE));
	if (m_Parser == NULL)
		m_Parser = new ROParser;

	m_GarbageLength = 0;
	m_PLT = PLT;
}

void ROPacket::Release()
{
	if (m_Garbage)
	{
		VirtualFree(m_Garbage, 0, MEM_RELEASE);
		m_Garbage = NULL;
	}
	if (m_Parser)
	{
		delete m_Parser;
		m_Parser = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////

// Recv

int ROPacket::Recv(char* Buffer, int Length)
{
	DbgPrintW(DBGINF, _CRT_WIDE("INF ROPacket::Recv Length=%d\n"), Length);

	PUCHAR InBuf = reinterpret_cast<PUCHAR>(Buffer);
	int InPos=0, InLen=0;

	if (m_GarbageLength == 0)
	{
		InLen = Length;

		if (g_Intrnl.Session.ConnFirst)
		{
			// Char/Zone接続直後のRecvでAIDが単品で来る
			// 他のパケットが後ろについてることもあるのでLength==4判定はx
			if (Length >= 4)
			{
				if (*reinterpret_cast<ULONG*>(InBuf) == g_Intrnl.Session.AID)
				{
					InPos += 4;
					InLen -= 4;
					g_Intrnl.Session.ConnFirst = false;
				}
			}

			// 0x0259, 0x0071での設定直後に来ると信じる
			//g_Intrnl.Session.ConnFirst = false;
		}
	}
	else
	{
		PUCHAR PtrNext = m_Garbage + m_GarbageLength;
		int ChkLen = m_GarbageLength + Length;

		if (ChkLen < 4)
		{
			RtlCopyMemory(PtrNext, InBuf, Length);
			m_GarbageLength += Length;
			return Length;
		}
		else
		{
			if (m_GarbageLength < 4)
				RtlCopyMemory(PtrNext, InBuf, 4-m_GarbageLength);

			int CurLen = GetLength(m_Garbage);
			if (CurLen == -1)
			{
				CurLen = *reinterpret_cast<USHORT*>(m_Garbage+2);
				if (CurLen == 0)
					CurLen = 2;
			}

			if (ChkLen < CurLen)
			{
				RtlCopyMemory(PtrNext, Buffer, Length);
				m_GarbageLength += Length;
				return Length;
			}
			else
			{
				RtlCopyMemory(PtrNext, InBuf, CurLen-m_GarbageLength);
				m_Parser->Recv(m_Garbage, CurLen);

				InPos = CurLen - m_GarbageLength;
				InLen = Length - InPos;

				m_GarbageLength = 0;
			}
		}
	}

	while (InLen > 1)
	{
		PUCHAR CurBuf = InBuf + InPos;

		int CurLen = GetLength(CurBuf);
		if (CurLen == -1)
		{
			if (InLen < 4)
				break;
			else
			{
				CurLen = *reinterpret_cast<USHORT*>(CurBuf+2);
				if (CurLen == 0)
					CurLen = 2;
			}
		}

		if (CurLen > InLen)
			break;
		else
		{
			m_Parser->Recv(CurBuf, CurLen);
			InPos += CurLen;
			InLen -= CurLen;
		}
	}

	if (InLen > 0)
	{
		RtlCopyMemory(m_Garbage, InBuf+InPos, InLen);
		m_GarbageLength = InLen;
	}

	return Length;
}

// Send

int ROPacket::Send(char* Buffer, int Length)
{
	return m_Parser->Send(reinterpret_cast<PUCHAR>(Buffer), Length);
}

////////////////////////////////////////////////////////////////////////////////


int ROPacket::GetLength(PUCHAR Buffer)
{
	if (m_PLT)
	{
		USHORT Op = *reinterpret_cast<USHORT*>(Buffer);

		int Length = GetLength(reinterpret_cast<PTR_PLT_ENTRY>(m_PLT)->Parent, Op);
		if (Length != 0)
			return Length;
	}

	return 2;
}

int ROPacket::GetLength(PTR_PLT_ENTRY Entry, ULONG Op)
{
	if (Entry)
	{
		if (Entry->Op == Op)
			return Entry->Length;
		else
		{
			if (Entry->Op > Op)
				return GetLength(Entry->Left, Op);
			else
				return GetLength(Entry->Right, Op);
		}
	}

	return 0;
}
