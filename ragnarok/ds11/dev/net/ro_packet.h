#ifndef RO_PACKET_H_INCLUDED
#define RO_PACKET_H_INCLUDED

#include "packet.h"
#include <windows.h>
#include <queue>
#include "ro_packet_enum.h"


class ROPacket : public Packet
{
	void* m_recv;
	void* m_send;
	DWORD m_plt;

public:

	ROPacket();
	virtual ~ROPacket();

public:

	void init(DWORD address);
	void init_api(void* recv, void* send);

private:

#pragma pack(push,1)
	typedef struct tagPLTNODE
	{
		struct tagPLTNODE* left;
		struct tagPLTNODE* parent;
		struct tagPLTNODE* right;
		u_long op;
		long length;
		u_long color;
	} NODE, *PNODE;
#pragma pack(pop)

private:

	void search_plt(DWORD address);
	int search_pltnode(NODE* node, u_short op);

	int get_length(NODE* node);
	int get_length(u_char* buf);

private:

	typedef struct tagPACKETQUE
	{
		int len;
		u_char* buf;
	} PQUE, *PPQUE;

	int m_sendQueLen;

	std::queue<PQUE> m_sendQue;
	std::queue<PQUE> m_recvQue;

private:

	void clear_sendque();
	void clear_recvque();

public:

	void send_f(u_char* buf, int buflen);
	void recv_f(u_char* buf, int buflen);
	int exec_sendque(SOCKET s, char* buf, int len, int flags);
	int exec_recvque(char* buf, int rest);
};


#endif	// #ifndef RO_PACKET_H_INCLUDED
