#ifndef NETBASE_HPP_3F0121D8_C486_401c_88F4_771280BFC85D
#define NETBASE_HPP_3F0121D8_C486_401c_88F4_771280BFC85D

#include <winsock2.h>

////////////////////////////////////////////////////////////////////////////////


class CNetBase
{
	u_char* m_backBuf;
	int m_backLen;
	int m_backCnt;

	enum { MAX_BUFFERSIZE=2048, };

protected:

	bool m_clear;

public:

	CNetBase();
	virtual ~CNetBase();

public:

	virtual int recv(char* buf, int len);
	virtual int send(const char* buf, int len);

private:

	void inc_backBuf(int require);
	void clear_backBuf();

private:

	virtual int parse_r(u_char* buf, int len) = 0;
	virtual int parse_s(u_char* buf, int len) = 0;
	virtual int getLength(u_char* buf) = 0;
};


#endif	// #ifndef NETBASE_HPP
