#include "packet.h"

////////////////////////////////////////////////////////////////////////////////


Packet::Packet()
	: m_backBuf(NULL), m_backLen(0), m_backSet(0)
{
	clr_backbuf();
}


Packet::~Packet()
{
	clr_backbuf();
}

////////////////////////////////////////////////////////////////////////////////


void Packet::inc_backbuf(int require)
{
	int rest = m_backSet*MAX_BUFFERSIZE - m_backLen;

	if (require > rest)
	{
		m_backSet++;

		u_char* p = new u_char[m_backSet*MAX_BUFFERSIZE];

		if (m_backBuf)
		{
			memcpy(p, m_backBuf, m_backLen);
			delete [] m_backBuf;
		}

		m_backBuf = p;
	}
}


void Packet::clr_backbuf()
{
	if (m_backBuf)
	{
		delete [] m_backBuf;
		m_backBuf = NULL;
	}

	m_backLen = 0;
	m_backSet = 0;
}

////////////////////////////////////////////////////////////////////////////////


int Packet::recv(char* buf, int buflen)
{
	u_char* parseBuf = reinterpret_cast<u_char*>(buf);
	int parsePos = 0;
	int parseRest = 0;

	if (m_backLen == 0)
	{
		parsePos = 0;
		parseRest = buflen;
	}
	else
	{
		bool result = prc_restbuf(reinterpret_cast<u_char*>(buf), buflen, &parsePos, &parseRest);
		if (result == false)
			return -1;
	}

	while (parseRest >= 2)
	{
		u_char* curbuf = parseBuf + parsePos;

		int curlen = get_length(curbuf);
		if (curlen == -1)
		{
			if (parseRest < 4)
				break;
			else
				curlen = *reinterpret_cast<u_short*>(curbuf+2);
		}

		if (curlen > parseRest)
			break;
		else
		{
			parse_r(curbuf, curlen);
			
			parsePos += curlen;
			parseRest -= curlen;
		}
	}

	if (parseRest > 0)
	{
		inc_backbuf(parseRest);
		memcpy(m_backBuf+m_backLen, parseBuf+parsePos, parseRest);
		m_backLen += parseRest;

		return -1;
	}
	else
	{
		if (m_backBuf)
			clr_backbuf();

		return 0;
	}
}


bool Packet::prc_restbuf(u_char* buf, int buflen, int* pos, int* rest)
{
	u_char* parseBuf = buf;

	if (m_backLen < 4)
		memcpy(m_backBuf+m_backLen, parseBuf, 4);

	int curlen = get_length(m_backBuf);
	if (curlen == -1)
		curlen = *reinterpret_cast<u_short*>(m_backBuf+2);

	int dfclen = curlen - m_backLen;
	if (dfclen > buflen)
	{
		// ç°âÒÇÃÇ≈Ç‡ë´ÇËÇ»Ç¢
		inc_backbuf(buflen);
		memcpy(m_backBuf+m_backLen, parseBuf, buflen);
		m_backLen += buflen;

		return false;
	}
	else
	{
		inc_backbuf(dfclen);
		memcpy(m_backBuf+m_backLen, parseBuf, dfclen);
		parse_r(m_backBuf, curlen);
		clr_backbuf();

		if (dfclen == buflen)
			return false;
		else
		{
			*pos = dfclen;
			*rest = buflen - dfclen;

			return true;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


int Packet::send(const char* buf, int buflen)
{
	return parse_s(reinterpret_cast<u_char*>(const_cast<char*>(buf)), buflen);
}
