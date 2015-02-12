#ifndef CLIENT_HPP_553D6702_5CAB_43fc_96ED_E9F3D988F919
#define CLIENT_HPP_553D6702_5CAB_43fc_96ED_E9F3D988F919

#include "../common.h"

////////////////////////////////////////////////////////////////////////////////


namespace client
{
	// patch.cpp

	void unprotect_sakray();
	void fix_hfont(enum ADDRESS type, void* font);

	// hook.cpp

	void apihook();
	void apihook_cleanup();

	// call.cpp

	enum TEXTCOLOR
	{
		// base

		CL_CYAN		= 0x00FFFF00,
		CL_RED		= 0x000000FF,
		CL_YELLOW	= 0x0000FFFF,
		CL_GREEN	= 0x0000FF00,
		CL_WHITE	= 0x00FFFFFF,
		CL_BLACK	= 0x00000000,

		// extent

		CL_TEXT_NOTICE1		= CL_CYAN,
		CL_TEXT_NOTICE2		= CL_RED,
		CL_TEXT_NOTICE3		= 0x00CEFF00,	// EQUIP系
		CL_TEXT_NOTICE4		= CL_YELLOW,	// commandメッセージ1
		CL_TEXT_NOTICE5		= 0x0063FFFF,	// commandメッセージ2
		CL_TEXT_NOTICE6		= 0x00FFE7E7,
		CL_TEXT_NORMAL		= CL_WHITE,
		CL_TEXT_NORMAL_OWN	= CL_GREEN,
		CL_TEXT_PT			= 0x00CECEFF,
		CL_TEXT_PT_OWN		= 0x0000CEFF,
		CL_TEXT_GUILD		= 0x00B5FFB5,
		CL_TEXT_BROADCAST	= CL_YELLOW,
		CL_TEXT_WHISPER		= CL_YELLOW,
		CL_TEXT_TALKIE		= 0x00FF8484,
		CL_TEXT_GUILDNOTICE	= 0x0084FFFF,
		CL_TEXT_NAME_MOB	= 0x00C6C6FF,
		CL_TEXT_NAME_NPC	= 0x00F7BF94,
		CL_TEXT_NAME_ITEM	= 0x0094EFFF,
	};

	HRESULT cl_ctprintf(const char* buf, COLORREF color);
	HRESULT cl_ctprintf(COLORREF color, const char* fmt, ...);

	HRESULT cl_loadgrf(const char* filename);
	
	// address.cpp

	enum ADDRESS {
		AD_AID,
		AD_GAME,
		AD_ACTOR,
		AD_FONT,
		AD_DD7,
		AD_CTRES,
		AD_NAME,
		AD_NET,
		AD_SEND,
		AD_RECV,
		AD_CL_CTPRINTF,
		AD_CL_LOADGRF,

		AD_MAX,
	};

	void search();
	DWORD address(enum ADDRESS type);

};


#endif	// #ifndef CLIENT_HP
