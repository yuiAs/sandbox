#include "packet.hpp"

////////////////////////////////////////////////////////////////////////////////


CPacket::CPacket()
	: m_backBuf(NULL), m_backLen(0), m_backCnt(0), m_table(0), m_fd(-1), m_breakSec(0), m_chat(0)
{
	clear_backBuf();
	m_table = new CPacketTable;

	chatlog_initialize();
}


CPacket::~CPacket()
{
	clear_backBuf();

	if (m_table)
		delete m_table;

	chatlog_finalize();
}

////////////////////////////////////////////////////////////////////////////////


int CPacket::recvCall(BYTE* __recvBuf, int __recvLen)
{
	BYTE* parseBuf = NULL;
	int parsePos=0, parseRest=0;
	
	if (m_backLen > 0)
	{
		if (m_backLen < 4)
			memcpy(m_backBuf+m_backLen, __recvBuf, 4);

		int currentLen = getLength(m_backBuf);
		if (currentLen == -1)
			currentLen = rev_cast<WORD>(m_backBuf+2);
		
		int deficiency =  currentLen - m_backLen;
		if (deficiency > __recvLen)
		{
			inc_backBuf(__recvLen);
			memcpy(m_backBuf+m_backLen, __recvBuf, __recvLen);
			m_backLen += __recvLen;

			return 0;
		}
		else
		{
			inc_backBuf(deficiency);
			memcpy(m_backBuf+m_backLen, __recvBuf, deficiency);
			parse(m_backBuf, -1);

			clear_backBuf();

			parseBuf = __recvBuf;
			parsePos = deficiency;
			parseRest = __recvLen - deficiency;
		}
	}
	else
	{
		parseBuf = __recvBuf;
		parsePos = 0;
		parseRest = __recvLen;
	}


	while (parseRest >= 2)
	{
		int currentLen = getLength(parseBuf+parsePos);
		if (currentLen == -1)
		{
			if (parseRest < 4)
				break;
			else
				currentLen = rev_cast<WORD>(parseBuf+parsePos+2);
		}
		if (currentLen > parseRest)
			break;

        parse(parseBuf+parsePos, currentLen);		
		parsePos += currentLen;
		parseRest -= currentLen;	// 要は parseRest = __recvLen - parsePos;
	}


	if (parseRest > 0)
	{
		// m_backBuf!=NULLでここまで来ることは事実上不可能

		if (m_backBuf == NULL)
		{
			m_backBuf = new BYTE[MAX_PACKET_BUFFER];
			m_backCnt = 1;

			memset(m_backBuf, 0, MAX_PACKET_BUFFER);
			memcpy(m_backBuf, parseBuf+parsePos, parseRest);
			m_backLen = parseRest;
		}
		else
		{
			inc_backBuf(parseRest);

			memcpy(m_backBuf+m_backLen, parseBuf+parsePos, parseRest);
			m_backLen += parseRest;
		}
	}
	else
	{
		if (m_backBuf)
			clear_backBuf();
	}

	return 0;
}


int CPacket::sendCall(BYTE* __sendBuf, int sendLen)
{
	parse(__sendBuf, sendLen);
	return 0;
}


int CPacket::getLength(BYTE* __buf)
{
	int length = m_table->get(rev_cast<WORD>(__buf));
	if (length == 0)
		return 2;

	return length;
}

////////////////////////////////////////////////////////////////////////////////


void CPacket::inc_backBuf(int __require)
{
	int length = MAX_PACKET_BUFFER * m_backCnt;
	
	if (__require > (MAX_PACKET_BUFFER*m_backCnt-m_backLen))
	{
		m_backCnt++;
		int length = MAX_PACKET_BUFFER * m_backCnt;

		BYTE* p = new BYTE[length];
		memset(p, 0, length);
		memcpy(p, m_backBuf, m_backLen);

		delete [] m_backBuf;				// 以前のbufferはdelete
		m_backBuf = p;
	}
}

void CPacket::clear_backBuf()
{
	if (m_backBuf || m_backLen)
		delete [] m_backBuf;

	m_backBuf = NULL;
	m_backLen = 0;
	m_backCnt = 0;
}
