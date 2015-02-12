#ifndef CONFIG_HPP_607FF8AD_2CD3_42da_A18E_48344170C36C
#define CONFIG_HPP_607FF8AD_2CD3_42da_A18E_48344170C36C

////////////////////////////////////////////////////////////////////////////////


namespace config
{
	enum key_bool
	{
		EX_SKIPMSG,
		EX_DBGLOG,
		SC_REGEX,
		FS_UNICODE,
		FONTA_ENABLE,
		FONTL_ENABLE,

		KEY_BOOL,
	};

	enum key_dword
	{
		FONTA_SIZE,
		FONTA_WEIGHT,
		FONTA_QUALITY,
		FONTL_SIZE,
		FONTL_WEIGHT,
		FONTL_QUALITY,
		FONTL_CHARSET,
		FONT_API,
		CHAT_FLAGS,
		PACKET_PLT,

		KEY_DWORD,
	};

	enum key_string
	{
		FONTA_NAME,
		FONTL_NAME,
	};

	enum chat_flags
	{
		CHAT_NONE=0x00,
		CHAT_NOR=0x01,
		CHAT_PRT=0x02,
		CHAT_GLD=0x04,
		CHAT_WIS=0x08,
		CHAT_GOD=0x10,
		CHAT_LBC=0x20,
	};

	////////////////////////////////////////////////////////////////////////////

	void load(const char* filename);
	void load_config();

	// interface

	bool enabled(enum key_bool type);
	unsigned int get_n(enum key_dword type);
	void get_s(enum key_string, char* buf, int buflen);

	// API Wrapper

	inline bool enabled(const char* sect, const char* key, int def=0);
	inline unsigned int get_n(const char* sect, const char* key, int def=0);
	inline bool get_s(const char* sect, const char* key, char* buf, int buflen, const char* def="");

};	// namespace config


#endif	// #ifndef CONFIG_HPP
