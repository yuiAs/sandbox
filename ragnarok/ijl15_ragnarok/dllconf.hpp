#ifndef DLLCONF_HPP
#define DLLCONF_HPP

#include "stdafx.h"


enum ADDR
{
	ADDR_GAMEBASE, ADDR_PUSHTEXT, ADDR_CMDJMPTBL,
	ADDR_NETBASE, ADDR_RECV, ADDR_SEND,
	ADDR_ACTORBASE, ADDR_AID, ADDR_CHARNAME,
	ADDR_XML_ADDRESS, ADDR_VDT,

	ADDR_END,
};


class DLLConf
{
	char* m_iniFile;
	DWORD m_addr[ADDR_END];

public:

	DLLConf() : m_iniFile(NULL) { memset(&m_addr, 0, sizeof(DWORD)*ADDR_END); }
	virtual ~DLLConf() { finalize(); }

public:

	void load(const char* filename);

	bool get_bool(const char* section, const char* key);
	int get_int(const char* section, const char* key);
	double get_double(const char* section, const char* key);
	size_t get_string(const char* section, const char* key, char* buf, size_t buflen);

	void set_addr(enum ADDR type, DWORD val) { m_addr[type] = val; }
	DWORD get_addr(enum ADDR type) { return m_addr[type]; }

private:

	void finalize();
};


#endif	// #ifndef DLLCONF_HPP
