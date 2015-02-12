#ifndef PACKET_HPP
#define PACKET_HPP

#include "chatlog.hpp"
#include "packettable.hpp"

////////////////////////////////////////////////////////////////////////////////


class CPacket
{
	enum { MAX_PACKET_BUFFER=2048, };

	BYTE* m_backBuf;
	int m_backLen;
	int m_backCnt;
	CPacketTable* m_table;

	int m_fd;
	unsigned long m_breakSec;
	ChatLog* m_chat;

public:

	CPacket();
	virtual ~CPacket();

private:

	template<typename T>
	inline T rev_cast(void *p) { return *reinterpret_cast<T*>(p); }

public:

	int recvCall(BYTE* __recvBuf, int __recvLen);
	int sendCall(BYTE* __sendBuf, int __sendLen);

public:

	void buildTable(DWORD address) { m_table->build(address); }

private:

	int parse(BYTE* __buf, int __buflen);
	int getLength(BYTE* __buf);

	void inc_backBuf(int __require);
	void clear_backBuf();

private:

	void chatlog_initialize();
	void chatlog_finalize();
	void chatlog_aquire(ULONG ltm);
	void chatlog_check();
	ULONG getLocalSeconds();
	ULONG getSpecificSeconds(void* ptf);	// void* = TIME_FIELDS*
	ULONG getSpecificSeconds(int year, int month, int day, int hour=0, int minute=0, int second=0);

private:

	bool isOwnAID(DWORD sid);

	void zc_change_option(BYTE* __buf);
	void zc_change_condition(BYTE* __buf);
};


#endif	// #ifndef PACKET_HPP
