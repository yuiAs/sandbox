#ifndef CLIENTCALL_HPP
#define CLIENTCALL_HPP

#include "dllconf.hpp"
extern DLLConf* conf;


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


void cl_pushText(const char* string, COLORREF color)
{
	DWORD addr_gamebase = conf->get_addr(ADDR_GAMEBASE);
	DWORD addr_call = conf->get_addr(ADDR_PUSHTEXT);

	if ((addr_gamebase==0) || (addr_call==0))
		return;

	void* base = reinterpret_cast<void*>(addr_gamebase);
	void* func = reinterpret_cast<void*>(addr_call);

	__asm
	{
		mov ecx, string
		push 0x00000000
		push color
		push ecx
		push 0x00000001
		mov ecx, [base]
		call [func]
	}
}


#endif	// #ifndef CLIENTCALL_HPP
