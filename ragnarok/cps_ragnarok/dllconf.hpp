#ifndef DLLCONF_HPP
#define DLLCONF_HPP

#include "stdafx.h"
#include "lua/lua.hpp"


enum ADDR
{
	ADDR_NETBASE, ADDR_GAMEBASE, ADDR_ACTORBASE, ADDR_PUSHTEXT,
	ADDR_AID,
	ADDR_XML_ADDRESS, ADDR_VDT,

	ADDR_END,
};


class DLLConf
{
	lua_State* m_lua;

	DWORD m_addr[ADDR_END];

public:

	DLLConf() : m_lua(0) {}
	virtual ~DLLConf() { finalize(); }

public:

	void load(const char* __luafile);

	bool get_bool(const char* __section, const char* __key);
	int get_int(const char* __section, const char* __key);
	size_t get_string(const char* __section, const char* __key, char* __buf, size_t __buflen);

	void set_addr(enum ADDR __type, DWORD __val) { m_addr[__type] = __val; }
	DWORD get_addr(enum ADDR __type) { return m_addr[__type]; }

private:

	void finalize();

	void from_priv(const char* __section, const char* __key);
	//void from_global(const char* __key);
};


#endif