#include "netbase.hpp"


////////////////////////////////////////////////////////////////////////////////


CNetBase::CNetBase()
	: m_backBuf(NULL), m_backLen(0), m_backCnt(0), m_clear(false)
{
	clear_backBuf();
}

CNetBase::~CNetBase()
{
	clear_backBuf();
}

////////////////////////////////////////////////////////////////////////////////


void CNetBase::inc_backBuf(int require)
{
	int rest = MAX_BUFFERSIZE*m_backCnt - m_backLen;

	if (require > rest)
	{
		m_backCnt++;

		u_char* p = new u_char[MAX_BUFFERSIZE*m_backCnt];
		memset(p, 0, MAX_BUFFERSIZE*m_backCnt);
		memcpy(p, m_backBuf, m_backLen);

		delete [] m_backBuf;
		m_backBuf = p;
	}
}


void CNetBase::clear_backBuf()
{
	if (m_backBuf)
		delete [] m_backBuf;

	m_backBuf = NULL;
	m_backLen = 0;
	m_backCnt = 0;

	m_clear = false;
}

////////////////////////////////////////////////////////////////////////////////


int CNetBase::recv(char* buf, int len)
{
	u_char* parseBuf = NULL;
	int parsePos=0, parseRest=0;

	if (m_clear)
		clear_backBuf();

	if (m_backLen > 0)
	{
		if (m_backLen < 4)
			memcpy(m_backBuf+m_backLen, buf, 4);

		int currentLen = getLength(m_backBuf);
		if (currentLen == -1)
			currentLen = *reinterpret_cast<u_short*>(m_backBuf+2);

		int deficiency =  currentLen - m_backLen;
		if (deficiency > len)
		{
			inc_backBuf(len);
			memcpy(m_backBuf+m_backLen, buf, len);
			m_backLen += len;

			return 0;
		}
		else
		{
			inc_backBuf(deficiency);
			memcpy(m_backBuf+m_backLen, buf, deficiency);
			parse_r(m_backBuf, currentLen);

			clear_backBuf();

			parseBuf = reinterpret_cast<u_char*>(buf);
			parsePos = deficiency;
			parseRest = len - deficiency;
		}
	}
	else
	{
		parseBuf = reinterpret_cast<u_char*>(buf);
		parsePos = 0;
		parseRest = len;
	}


	while (parseRest >= 2)
	{
		int currentLen = getLength(parseBuf+parsePos);
		if (currentLen == -1)
		{
			if (parseRest < 4)
				break;
			else
				currentLen = *reinterpret_cast<u_short*>(parseBuf+parsePos+2);
		}

		if (currentLen > parseRest)
			break;
		else
		{
			parse_r(parseBuf+parsePos, currentLen);
			parsePos += currentLen;
			parseRest -= currentLen;
		}
	}


	if (parseRest > 0)
	{
		if (m_backBuf)
		{
			inc_backBuf(parseRest);

			memcpy(m_backBuf+m_backLen, parseBuf+parsePos, parseRest);
			m_backLen += parseRest;
		}
		else
		{
			m_backBuf = new u_char[MAX_BUFFERSIZE];
			m_backLen = parseRest;
			m_backCnt = 1;

			memset(m_backBuf, 0, MAX_BUFFERSIZE);
			memcpy(m_backBuf, parseBuf+parsePos, parseRest);
		}
	}
	else
	{
		if (m_backBuf)
			clear_backBuf();
	}

	return 0;
}


int CNetBase::send(const char* buf, int len)
{
	return parse_s(reinterpret_cast<BYTE*>(const_cast<char*>(buf)), len);
}
