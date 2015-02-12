#include "dllconf.hpp"


////////////////////////////////////////////////////////////////////////////////


void DLLConf::finalize()
{
	if (m_lua)
		lua_close(m_lua);
}


void DLLConf::load(const char* __luafile)
{
	if (m_lua == 0)
		m_lua = lua_open();

	luaL_loadfile(m_lua, __luafile);
	lua_pcall(m_lua, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////


void DLLConf::from_priv(const char* __section, const char* __key)
{
	lua_pushstring(m_lua, __section);		// push
	lua_gettable(m_lua, LUA_GLOBALSINDEX);	// pop -> push
	lua_pushstring(m_lua, __key);			// push
	lua_gettable(m_lua, -2);				// pop -> push
}


bool DLLConf::get_bool(const char* __section, const char* __key)
{
	if (__section != NULL)
		from_priv(__section, __key);
	else
		lua_getfield(m_lua, LUA_GLOBALSINDEX, __key);

	bool result = lua_toboolean(m_lua, -1)==1;
	// toboolean‚Ífalse/nilˆÈŠO‚Í‚·‚×‚Ä1‚ð•Ô‚·
	lua_settop(m_lua, -2);

	return result;
}


int DLLConf::get_int(const char* __section, const char* __key)
{
	if (__section != NULL)
		from_priv(__section, __key);
	else
		lua_getfield(m_lua, LUA_GLOBALSINDEX, __key);

	int result = lua_tointeger(m_lua, -1);
	lua_settop(m_lua, -2);

	return result;
}


size_t DLLConf::get_string(const char* __section, const char* __key, char* __buf, size_t __buflen)
{
	if (__section != NULL)
		from_priv(__section, __key);
	else
		lua_getfield(m_lua, LUA_GLOBALSINDEX, __key);


	size_t length = lua_objlen(m_lua, -1);

	if (__buf != NULL)
	{
		if (lua_type(m_lua, -1) == LUA_TSTRING)
			memcpy(__buf, lua_tolstring(m_lua, -1, NULL), __buflen);
		else
			length = 0;
	}

	lua_settop(m_lua, -2);

	return length;
}
