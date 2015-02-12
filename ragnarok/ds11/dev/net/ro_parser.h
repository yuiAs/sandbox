#ifndef RO_PARSER_H
#define RO_PARSER_H

#include "ro_packet.h"
#include "ro_log.h"


class ROPacketParser : public ROPacket
{
	ROLog* m_log;

	u_long m_aid;
	u_short m_elementCI;
	u_char m_truesight;

	// config
	
	bool m_blockSTUP;
	bool m_blockSKUP;
	bool m_fixDir;

public:

	ROPacketParser();
	~ROPacketParser();

public:

	void init(DWORD address);

private:

	void init_val();
	void destroy();

private:

	int parse_r(u_char* buf, int buflen);
	int parse_s(u_char* buf, int buflen);

private:

	bool is_ownAID(const u_long sid);
	bool is_NumLockOFF();

private:

	void push_stateNotice(u_char flag, char* target);
	void push_elementNotice(u_char flag, char* element);
	void push_condition(u_short type, u_char flag);

	void push_elementConv(u_short type, char* element);
	void release_elementConv(u_char* buf);

	void fix_direction(u_char* buf, int head, int body);

private:

	void cl_enter_ack();
	void cl_option(u_char* buf);
	void cl_option_own(u_char* buf);
	void cl_condition(u_char* buf);
	void cl_useitem_ack(u_char* buf);

private:

	void local_debug();
	bool local_truesight();
	bool local_stup();
	bool local_skup();
};


#endif	// #ifndef RO_PARSER_H
