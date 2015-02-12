#ifndef CLIENTENUM_H
#define CLIENTENUM_H

////////////////////////////////////////////////////////////////////////////////

// MSGTABLE

typedef enum _MSGTABLE_ID
{
	MSG_NOMSG=0,

	MSG_WHIS_FAILED=148,
	MSG_WHIS_REFUSE=150,

	MSG_RESTORE_WEAPON_ELEMENT=471,
	MSG_RESTORE_ARMOR_ELEMENT=473,
} MSGTABLE_ID;

////////////////////////////////////////////////////////////////////////////////

// STRCOLOR

typedef enum _STRCOLOR_ID
{
	COL_CYAN	= 0x00FFFF00,
	COL_RED		= 0x000000FF,
	COL_YELLOW	= 0x0000FFFF,
	COL_GREEN	= 0x0000FF00,
	COL_WHITE	= 0x00FFFFFF,
	COL_BLACK	= 0x00000000,

	COL_NOTICE1		= COL_CYAN,
	COL_NOTICE2		= COL_RED,
	COL_NOTICE3		= 0x00CEFF00,	// EQUIP系
	COL_NOTICE4		= COL_YELLOW,	// commandメッセージ1
	COL_NOTICE5		= 0x0063FFFF,	// commandメッセージ2
	COL_NOTICE6		= 0x00FFE7E7,
	COL_NORMAL		= COL_WHITE,
	COL_NORMAL_OWN	= COL_GREEN,
	COL_PT			= 0x00CECEFF,
	COL_PT_OWN		= 0x0000CEFF,
	COL_GUILD		= 0x00B5FFB5,
	COL_BROADCAST	= COL_YELLOW,
	COL_WHISPER		= COL_YELLOW,
	COL_TALKIE		= 0x00FF8484,
	COL_GUILDNOTICE	= 0x0084FFFF,
	COL_NAME_MOB	= 0x00C6C6FF,
	COL_NAME_NPC	= 0x00F7BF94,
	COL_NAME_ITEM	= 0x0094EFFF,
} STRCOLOR_ID;

////////////////////////////////////////////////////////////////////////////////

// EFFECT

typedef enum _EFFECT_ID
{
	EC_NONE=0x0B,

	EC_STORMGUST=0x59,
	EC_VERMILION,
	EC_METEOR=0x5C,
	EC_QUAMIRE=0x5F,
	EC_FIREPILLAR,
	EC_FIREPILLAR_BOMB,
} EFFECT_ID;

////////////////////////////////////////////////////////////////////////////////

#endif	// #ifndef CLIENTENUM_H
