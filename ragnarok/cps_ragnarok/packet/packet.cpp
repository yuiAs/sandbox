#include "packet.hpp"


////////////////////////////////////////////////////////////////////////////////


CPacket::CPacket()
	: m_backBuf(NULL), m_backLen(0), m_backCnt(0)
{
}


CPacket::~CPacket()
{
	clear_backBuf();
}

////////////////////////////////////////////////////////////////////////////////


int CPacket::parse(BYTE* __buf, int __buflen)
{
	if (__buflen > 0)
	{
		int length = getLength(__buf);
		if (length > __buflen)
			return -1;
	}

	switch (rev_cast<WORD>(__buf))
	{
		default:
			break;
	}

	return __buflen;
}

////////////////////////////////////////////////////////////////////////////////


int CPacket::recvCall(BYTE* __recvBuf, int __recvLen)
{
	BYTE* parseBuf = NULL;
	int parsePos=0, parseLen=0, parseRest=0;
	
	if (m_backLen > 0)
	{
		int deficiency = getLength(m_backBuf) - m_backLen;
		if (deficiency > __recvLen)
		{
			inc_backBuf();

			memcpy(m_backBuf+m_backLen, __recvBuf, __recvLen);
			m_backLen += __recvLen;

			return __recvLen;
		}
		else
		{
			if (deficiency > (MAX_PACKET_BUFFER*m_backCnt-m_backLen))
				inc_backBuf();

			memcpy(m_backBuf+m_backLen, __recvBuf, deficiency);
			parse(m_backBuf, -1);

			clear_backBuf();

			parseBuf = __recvBuf;
			parseLen = __recvLen;
			parsePos = deficiency;
			parseRest = __recvLen - deficiency;
		}
	}
	else
	{
		parseBuf = __recvBuf;
		parseLen = __recvLen;
		parsePos = 0;
		parseRest = __recvLen;
	}


	while (parseRest > 1)
	{
		int currentLen = parse(parseBuf+parsePos, parseRest);
		if (currentLen == -1)
			break;
		
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
			if ((m_backLen+parseRest) > MAX_PACKET_BUFFER)
				inc_backBuf();

			memcpy(m_backBuf+m_backLen, parseBuf+parsePos, parseRest);
			m_backLen += parseRest;
		}
	}
	else
	{
		clear_backBuf();
	}

	return __recvLen;
}


int CPacket::sendCall(BYTE* __sendBuf, int sendLen)
{
	parse(__sendBuf, -1);
	return sendLen;
}


int CPacket::getLength(BYTE* __buf)
{
	int length = 0;

	if (length == 0)
		return 2;
	if (length == -1)
		return rev_cast<WORD>(__buf+2);

	return length;
}

////////////////////////////////////////////////////////////////////////////////

/*
void CPacket::incBuffer(int __newlen, BYTE* __buf, int __buflen)
{
	BYTE* p = new BYTE[__newlen];
	memset(p, 0, __newlen);

	if (__buflen)
	{
		memcpy(p, __buf, __buflen);
		delete [] __buf;
	}

	__buf = p;
}
*/

void CPacket::inc_backBuf()
{
	m_backCnt++;
	int length = MAX_PACKET_BUFFER * m_backCnt;

	BYTE* p = new BYTE[length];
	memset(p, 0, length);
	memcpy(p, m_backBuf, m_backLen);

	delete [] m_backBuf;				// 以前のbufferはdelete
	m_backBuf = p;
}

void CPacket::clear_backBuf()
{
	if (m_backBuf)
		delete [] m_backBuf;

	m_backBuf = NULL;
	m_backLen = 0;
	m_backCnt = 0;
}
