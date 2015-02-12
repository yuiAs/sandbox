#ifndef PACKETPARSE_HPP
#define PACKETPARSE_HPP


#include <windows.h>
#include <map>
#include <bitset>
#include "../Ragnarok.hpp"
#include "../outputFile.hpp"
#include "chatLog.hpp"


class CPacketParser
{

protected:

	// define const

	enum
	{
		NONE=0,

		MAX_PACKET_SIZE=2048,				// clientÇ™ämï€ÇµÇƒÇÈBuffer size
		MAX_PACKETBUFFER_SIZE=65536,		// classì‡èàóùópBuffer size

		SEND_PERMIT=0,
		SEND_FORBIDDEN=1, 

		// CharServer
		CHARDATA_LENGTH=106,				// CharacterData size
		KRO_CS_PADDING=20,					// kROâ€ã‡èÓïÒ

		// ZoneServer
		PACKET_SZ_CHANGECONDITION=9,
		PACKET_OP_CHANGECONDITION=0x0196,
	};

	enum
	{
		PACKET_FLAG=0,

		CH_SYSTEM=1, CH_NORMAL, CH_BROADCAST, CH_LOCALBC, CH_PARTY, CH_GUILD, CH_WHISPER, CH_TALKIE,

		NATURAL_CHAT=17,
		PACKET_DUMP, EFFECTIVE_MSG, ADVANCED_MSG, DEAD_INFO, THROUGH_ITEM, BLOCK_TRADE_REQ,

		PACKET_ZS_CHANGEMAP, PACKET_ZS_CHANGEMAP_WO_ZONE,
		PACKET_ZS_WAITFIRSTTICK, PACKET_ZS_TELEPORT_RANDOM, PACKET_ZS_TELEPORT_ITEM, PACKET_ZS_INTIMIDATE,

		PACKET_FLAG_END
	};


protected:

	CRagnarok* m_core;
	CChatLog* m_chat;
	COutputFile* m_plog;

	std::bitset<PACKET_FLAG_END>* m_flag;

private:

	BYTE* m_packetBuf;
	int m_packetBuf_length;
	int* m_table;
	DWORD m_lastOp;

	TCHAR* m_currentMap;
	unsigned long m_currentIP;

	typedef std::pair<DWORD, ACTOR_DATA> m_pair_cs;
	std::map<DWORD, ACTOR_DATA> m_cs;

protected:

	template<typename T>
	inline T p_cast(void *p) { return *reinterpret_cast<T*>(p); }

public:

	CPacketParser();
	~CPacketParser();

	void buildTable(DWORD address, int pos);
	bool isTable() { return m_table!=0; }

private:

	void initialize();

protected:

	int length(WORD op) const 
	{
		if (op > m_lastOp)
			return 1;
		if (m_table[op] == 0)
			return 2;

		return m_table[op];
	}

	void dump(BYTE* buf, int length, bool recv);

public:

	int parseRecv(BYTE* buf, int length);
	int parseSend(BYTE* buf, int length);

	void setCurrentIP(unsigned long addr) { m_currentIP = addr; }

private:

	virtual int parse_recv(BYTE* rwbuf, int length);
	virtual int parse_send(BYTE* rwbuf, int length);

protected:

	int recv_svinfo(BYTE* buf, int length);
	int recv_charinfo(BYTE* buf, int length);
	int recv_charmakingsucceeded(BYTE* buf, int length);
	int recv_zoneinfo(BYTE* buf, int length);
	int recv_connectzone(BYTE* buf, int length);
	int recv_changemap(BYTE* buf, int length);
	int recv_changezone(BYTE* buf, int length);
	int recv_normalchat(BYTE* buf, int length);
	int recv_normalchat_own(BYTE* buf, int length);
	int recv_whisper(BYTE* buf, int length);
	int recv_broadcast(BYTE* buf, int length);
	int recv_ptchat(BYTE* buf, int length);
	int recv_guildchat(BYTE* buf, int length);
	int recv_talkie(BYTE* buf, int length);
	int recv_localbroadcast(BYTE* buf, int length);
	int recv_mvp(BYTE* buf, int length);
	int recv_updatestatus_1(BYTE* buf, int length);
	int recv_updatestatus_2(BYTE* buf, int length);
	int recv_changeoption(BYTE* buf, int length);
	int recv_changecondition(BYTE* buf, int length);

	int send_reqtrading(BYTE* buf, int length);
	int send_whisper(BYTE* buf, int length);
	int send_ptchat(BYTE* buf, int length);
	int send_guildchat(BYTE* buf, int length);
};


#endif	// #ifndef PACKETPARSE_HPP
