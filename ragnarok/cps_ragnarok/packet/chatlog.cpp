#include "chatlog.hpp"

////////////////////////////////////////////////////////////////////////////////


void ChatLog::initialize(const char* __luafile)
{
	initialize_lua(__luafile);
}

////////////////////////////////////////////////////////////////////////////////


void ChatLog::initialize_lua(const char* __luafile)
{
	if (m_lua == 0)
		m_lua = lua_open();
	
	luaopen_base(m_lua);
	luaopen_string(m_lua);	// string.format()
	luaopen_os(m_lua);		// os.data()

	//luaL_dofile(m_lua, __luafile);
	luaL_loadfile(m_lua, __luafile);
	lua_pcall(m_lua, 0, 0, 0);
}


void ChatLog::finalize_lua()
{
	if (m_lua)
		lua_close(m_lua);
}

void ChatLog::output(const char* __function, const char* __name, const char* __body)
{
	//lua_getglobal(m_lua, __function);
	lua_getfield(m_lua, LUA_GLOBALSINDEX, __function);
	lua_pushstring(m_lua, __name);
	lua_pushstring(m_lua, __body);
	lua_call(m_lua, 2, 1);

	// èoóÕ

	if (lua_type(m_lua, -1) == LUA_TSTRING)
	{
	}

	lua_settop(m_lua, -2);
}
