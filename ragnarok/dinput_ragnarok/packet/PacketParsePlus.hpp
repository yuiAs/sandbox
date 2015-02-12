#ifndef PACKETPARSEPLUS_HPP
#define PACKETPARSEPLUS_HPP


#include "PacketParse.hpp"
#include <set>


class CPacketParserPlus : public CPacketParser
{
	typedef std::pair<DWORD, WORD> m_pair_npc;
	std::map<DWORD, WORD> m_npc;
	std::set<WORD> m_npc_dead;

public:

	CPacketParserPlus()
	{
		m_npc.clear();
		initialize_plus();
	}

	~CPacketParserPlus();

private:

	void initialize_plus();

private:

	virtual int parse_recv(BYTE* rwbuf, int length);

	int recv_dropitem_m(BYTE* buf, int length);
	int recv_dropitem_s(BYTE* buf, int length);
	int recv_addactor(BYTE* buf, int length);
	int recv_eraseactor(BYTE* buf, int length);
};

#endif	// #ifndef PACKETPARSEPLUS_HPP