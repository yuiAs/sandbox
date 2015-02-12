#include <windows.h>
#include "config.hpp"


namespace config
{
	static char m_filename[MAX_PATH];

	static bool m_bool[KEY_BOOL];
	static unsigned int m_dword[KEY_DWORD];

////////////////////////////////////////////////////////////////////////////////


void load(const char* filename)
{
	::lstrcpy(m_filename, filename);
}

void load_config()
{
	memset(m_bool, false, sizeof(m_bool));
	memset(m_dword, 0, sizeof(m_dword));

	m_bool[EX_SKIPMSG] = enabled("extention", "skip_errormsg");
	m_bool[EX_DBGLOG] = enabled("extention", "debug_log");
	m_bool[SC_REGEX] = enabled("shortcut", "reg_extended");
	m_bool[FS_UNICODE] = enabled("filesystem", "support_unicode");

	m_dword[PACKET_PLT] = get_n("packet", "plt_address");
}

////////////////////////////////////////////////////////////////////////////////

// interface

bool enabled(enum key_bool type)
{
	return m_bool[type];
}

unsigned int get_n(enum key_dword type)
{
	return m_dword[type];
}

void get_s(enum key_string type, char* buf, int buflen)
{
	switch (type)
	{
	}
}

////////////////////////////////////////////////////////////////////////////////

// API Wrapper

inline bool enabled(const char* sect, const char* key, int def)
{
	return ::GetPrivateProfileInt(sect, key, def, m_filename)!=def;
}

inline unsigned int get_n(const char* sect, const char* key, int def)
{
	return ::GetPrivateProfileInt(sect, key, def, m_filename);
}

inline bool get_s(const char* sect, const char* key, char* buf, int buflen, const char* def)
{
	return ::GetPrivateProfileString(sect, key, def, buf, buflen, m_filename)!=0;
}

// quality
// 3	NONANTIALIASED_QUALITY
// 4	ANTIALIASED_QUALITY
// 5	CLEARTYPE_QUALITY

};	// namespace config
