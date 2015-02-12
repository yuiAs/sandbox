#include "../common.h"
#include "network.hpp"
#include <queue>


namespace network
{
	typedef struct tagPACKETQUE
	{
		int len;
		u_char* buf;
	} PQUE, *LPPQUE;

	std::queue<PQUE> m_sbuf;
	std::queue<PQUE> m_rbuf;

////////////////////////////////////////////////////////////////////////////////


void sbuf_push(u_char* buf, int buflen)
{
	PQUE t = { buflen, buf };
	m_sbuf.push(t);
}

void sbuf_clear()
{
	while (m_sbuf.size())
	{
		PQUE t = m_sbuf.front();
		SAFE_DELETE_ARRAY(t.buf);
		m_sbuf.pop();
	}
}

void sbuf_send(SOCKET s, int flags)
{
	while (m_sbuf.size())
	{
		PQUE t = m_sbuf.front();
		if (t.buf)
		{
			dbgprintf(0, "sbuf_send op=%04X len=%d\n", _mkU16(t.buf), t.len);

			typedef int (WSAAPI *WS2_SEND)(SOCKET, const char*, int, int);
			reinterpret_cast<WS2_SEND>(pfn_ws2_send)(s, reinterpret_cast<char*>(t.buf), t.len, flags);
			SAFE_DELETE_ARRAY(t.buf);
		}
		m_sbuf.pop();
	}
}

////////////////////////////////////////////////////////////////////////////////


void rbuf_push(u_char* buf, int buflen)
{
	PQUE t = { buflen, buf };
	m_rbuf.push(t);
}

void rbuf_clear()
{
	while (m_rbuf.size())
	{
		PQUE t = m_rbuf.front();
		SAFE_DELETE_ARRAY(t.buf);
		m_rbuf.pop();
	}
}

int rbuf_send(u_char* buf, int rest)
{
	if (rest < 0)
		return 0;

	int result = 0;

	while (m_rbuf.size())
	{
		PQUE t = m_rbuf.front();
		
		if ((result+t.len) > rest)
			break;
		else
		{
			if (t.buf)
			{
				dbgprintf(0, "rbuf_send op=%04X len=%d\n", _mkU16(t.buf), t.len);

				memcpy(buf+result, t.buf, t.len);
				delete [] t.buf;

				result += t.len;
			}
			m_rbuf.pop();
		}
	}

	return result;
}


};	// namespace network