#ifndef CLIENT_ENUM_H_INCLUDED
#define CLIENT_ENUM_H_INCLUDED


enum CLIENT_CONST_VALUE
{
	LINEBUF_SIZE=80,
};

////////////////////////////////////////////////////////////////////////////////


enum TEXT_COLOR
{
	// base

	CL_CYAN		= 0x00FFFF00,
	CL_RED		= 0x000000FF,
	CL_YELLOW	= 0x0000FFFF,
	CL_GREEN	= 0x0000FF00,
	CL_WHITE	= 0x00FFFFFF,
	CL_BLACK	= 0x00000000,

	// extent

	CL_TXT_NOTICE1		= CL_CYAN,
	CL_TXT_NOTICE2		= CL_RED,
	CL_TXT_NOTICE3		= 0x00CEFF00,	// EQUIP系
	CL_TXT_NOTICE4		= CL_YELLOW,	// commandメッセージ1
	CL_TXT_NOTICE5		= 0x0063FFFF,	// commandメッセージ2
	CL_TXT_NOTICE6		= 0x00FFE7E7,
	CL_TXT_NORMAL		= CL_WHITE,
	CL_TXT_NORMAL_OWN	= CL_GREEN,
	CL_TXT_PT			= 0x00CECEFF,
	CL_TXT_PT_OWN		= 0x0000CEFF,
	CL_TXT_GUILD		= 0x00B5FFB5,
	CL_TXT_BROADCAST	= CL_YELLOW,
	CL_TXT_WHISPER		= CL_YELLOW,
	CL_TXT_TALKIE		= 0x00FF8484,
	CL_TXT_GUILDNOTICE	= 0x0084FFFF,
	CL_TXT_NAME_MOB		= 0x00C6C6FF,
	CL_TXT_NAME_NPC		= 0x00F7BF94,
	CL_TXT_NAME_ITEM	= 0x0094EFFF,
};

////////////////////////////////////////////////////////////////////////////////


enum MESSAGE_TABLE
{
	NO_MSG=0,

	MSG_WIS_FAILED=148,
	MSG_WIS_REFUSE=150,

	MSG_RESTORE_WEAPON_ELEMENT=471,
};

////////////////////////////////////////////////////////////////////////////////


enum ADDRESS
{
	AD_NETBASE,
	AD_RECV,
	AD_SEND,
	AD_PLT,
	AD_AID,
	AD_CHARNAME,
	AD_ACTORBASE,
	AD_GAMEBASE,
	AD_DRAWBASE,
	AD_ZONE_JMPTBL,
	AD_CALL_CLPRT,
	AD_CALL_MSGTBL,
	AD_CALL_ZP0196,

	ADDRESS_MAX,
};


#endif	// CLIENT_ENUM_H_INCLUDED
