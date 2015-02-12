#ifndef CHATLOG_HPP
#define CHATLOG_HPP

#include "../lua/lua.hpp"


class ChatLog
{
	lua_State* m_lua;

public:

	ChatLog() : m_lua(0) {}
	virtual ~ChatLog() { finalize_lua(); }

	void initialize(const char* __luafile);

private:

	// luaëÄçÏån

	void initialize_lua(const char* __luafile);
	void finalize_lua();

	void output(const char* __function, const char* __name, const char* __body);

public:

	void normal(const char* __body) { output("chat_normal", NULL, __body); }
	void party(const char* __body) { output("chat_party", NULL, __body); }
	void guild(const char* __body) { output("chat_guild", NULL, __body); }
	void wis_s(const char* __name, const char* __body) { output("chat_wis_s", __name, __body); }
	void wis_r(const char* __name, const char* __body) { output("chat_wis_r", __name, __body); }
	void broadcast(const char* __body) { output("chat_broadcast", NULL, __body); }
	void lbc(const char* __body) { output("chat_lbc", NULL, __body); }
	void talkie(const char* __body) { output("chat_talkie", NULL, __body); }
	
};


#endif	// #ifndef CHATLOG_HPP
