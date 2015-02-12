#ifndef PACKET_HPP
#define PACKET_HPP

#include "netcode.hpp"

////////////////////////////////////////////////////////////////////////////////


class CPacket
{
	enum { MAX_PACKET_BUFFER=2048, };
	
	BYTE* m_backBuf;
	int m_backLen;
	int m_backCnt;

public:

	CPacket();
	virtual ~CPacket();

private:

	template<typename T>
	inline T rev_cast(void *p) { return *reinterpret_cast<T*>(p); }

public:

	int recvCall(BYTE* __recvBuf, int __recvLen);
	int sendCall(BYTE* __sendBuf, int __sendLen);

private:

	int parse(BYTE* __buf, int __buflen);
	int getLength(BYTE* __buf);

	//void incBuffer(int __newlen, BYTE* __buf, int __buflen);
	void inc_backBuf();
	void clear_backBuf();
};


#endif	// #ifndef PACKET_HPP
