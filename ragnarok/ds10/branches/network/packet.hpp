#ifndef PACKET_HPP_7E4CA2BA_5B92_473d_9836_FE31B9322481
#define PACKET_HPP_7E4CA2BA_5B92_473d_9836_FE31B9322481

#include "netbase.hpp"
#include "chatlog.hpp"

////////////////////////////////////////////////////////////////////////////////


class CPacket : public CNetBase
{
	enum 
	{
		SEARCH_RANGE=0x0002FFFF,
		FAILED_VALUE=0,

		MAX_CIID=0xFF,
	};

	DWORD m_root;

	ChatLog* m_chat;
	DWORD m_chatFlag;

	u_char m_truesight;
	u_short m_currentCI;
	bool m_blockSKUp;
	bool m_blockSTUp;

public:

	CPacket();
	~CPacket();

public:

	void initialize(u_long address, bool as=true);

private:

	void loadConfig();

private:

	typedef struct tagNODE
	{
		struct tagNODE* left;
		struct tagNODE* parent;
		struct tagNODE* right;
		u_long op;
		long length;
		u_long color;
	} NODE, *PNODE;

	void search(u_long address);
	int getLength(NODE* node, u_short op);

private:

	// for parser

	inline bool isOwnAID(u_long sid);
	inline bool isSHIFT();
	inline bool isNumLockOFF();

	void push_changeCondition(u_short type, u_char flag);

	void changeState(u_char flag, char* state);
	void changeElement(u_char flag, char* element);
	void releaseElementalConv(u_char* buf);

	void cl_change_option(u_char* buf);
	void cl_change_option_own(u_char* buf);
	void cl_change_condition(u_char* buf);
	void cl_notice_actor(u_char* buf);
	void cl_notice_useitem(u_char* buf);

	void cl_changedir_fix(u_char* buf);

	void local_debugMsg();
	bool local_trueSight();
	bool local_0x00BB();
	bool local_0x0112();

//	void local_debugLog1(u_char* buf, int len);
//	void local_debugLog2(u_char* buf, int len);

private:

	int parse_r(u_char* buf, int len);
	int parse_s(u_char* buf, int len);
	int getLength(u_char* buf);
};


#endif	// #ifndef PACKET_HPP
