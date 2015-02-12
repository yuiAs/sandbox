#ifndef RAGNAROK_H
#define RAGNAROK_H


#include <tchar.h>	// TCHAR

////////////////////////////////////////////////////////////////////////////////

// string

static const TCHAR* GLOBALNAME = _T("Global\\ONEDAY_CALLED_GENPATSU_NOTTORI");

////////////////////////////////////////////////////////////////////////////////

// enum

// 配列用

enum ADDR
{
	AD_GAMEBASE,
	AD_ACTORBASE,
	AD_CALL_PUSHTEXT,
	AD_CALL_SHSCWND, 
	AD_SCWNDFLAG,
	AD_SKILLID, 
	AD_NPCID,
	AD_SLBASE,
	AD_SNBASE,
	AD_SCPAGE,
	AD_ACCSVIP,
	AD_VDT,
	AD_GETPROCADDR,
	
	ENUM_ADDR
};

// 制御用イベント

enum SIGNAL { SL_ZONE, SL_CHAR, SL_FINALIZE, ENUM_SL };

// 固定長サイズ指定

enum
{
	LN_TEXT_SYSTEM=70,

	LN_NAME_CHAR=24,
	LN_NAME_NPC=24,
	LN_NAME_MAP=16,
	LN_NAME_SERVER=20,

	LN_NET_IP=16,			// aaa.bbb.ccc.ddd
	LN_NET_HOSTNAME=260,

	LN_SKILLID=0x20,
};

// 色設定

enum
{
	CL_CYAN		= 0x00FFFF00,
	CL_RED		= 0x000000FF,
	CL_YELLOW	= 0x0000FFFF,
	CL_GREEN	= 0x0000FF00,
	CL_WHITE	= 0x00FFFFFF,
	CL_BLACK	= 0x00000000,

	// RAGNAROK_CLIENT

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

////////////////////////////////////////////////////////////////////////////////

// struct

// ACTOR、要するにPC/NPCオブジェクト用
// 使いまわしたいのでCRagnarokの外で定義

enum ACTOR
{
	ACTOR_SID,
	ACTOR_CID,
	ACTOR_BASEEXP,
	ACTOR_BASENEXT,
	ACTOR_JOBEXP,
	ACTOR_JOBNEXT,
	ACTOR_CURHP,
	ACTOR_MAXHP,
	ACTOR_CURSP,
	ACTOR_MAXSP,
	ACTOR_ZENY,
	ACTOR_CLASS,
	ACTOR_BASELV,
	ACTOR_JOBLV,

	ENUM_ACTOR
};

typedef struct
{
	unsigned long value[ENUM_ACTOR];
	TCHAR name[LN_NAME_CHAR];
} ACTOR_DATA;


#endif	// #ifndef RAGNAROK_H

