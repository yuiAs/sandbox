#include "../clientcall.hpp"
#include "packet.hpp"
#include "def_packet.h"

////////////////////////////////////////////////////////////////////////////////


int CPacket::parse(BYTE* __buf, int __buflen)
{
	switch (rev_cast<WORD>(__buf))
	{
//		case PACKET_AC_AUTH_SUCCEEDED:
//			//DbgPrint(_T("PACKET_AC_AUTH_SUCCEEDED aid=%08X"), rev_cast<DWORD>(__buf+8));
//			break;

		case PACKET_CC_SELECT_SUCCEEDED:
			m_chat->sys_zone(__buf+6);
			break;

		case PACKET_ZC_CONNECT_SUCCEEDED:
			clear_backBuf();
			// この時点でcharnameはmemory上にあるはず
			if (DWORD address = conf->get_addr(ADDR_CHARNAME))
				m_chat->sys_char(reinterpret_cast<BYTE*>(address));
			break;

		case PACKET_ZC_CHAT_NORMAL:
			chatlog_check();
			m_chat->normal(__buf+8);
			break;

		case PACKET_ZC_CHAT_NORMAL_OWN:
			chatlog_check();
			m_chat->normal(__buf+4);
			break;

		case PACKET_ZC_CHANGE_MAP_WITH_ZONE:
			m_chat->sys_zone(__buf+2);
			break;

		case PACKET_CZ_WHISPER:
			chatlog_check();
			m_chat->wis_s(__buf+4, __buf+28);
			break;

		case PACKET_ZC_WHISPER:
			chatlog_check();
			// [op:02][len:02][name:24] 28 29 30 31 [body??]
			if (rev_cast<WORD>(__buf+2) > 32)
			{
				// 0x00FFFFFF以上のAIDは存在しないと仮定
				if (rev_cast<BYTE>(__buf+31) == 0x00)
					m_chat->wis_r(__buf+4, __buf+28+4);
				else
					m_chat->wis_r(__buf+4, __buf+28);
			}
			else
				// len<=32は必然的に旧
				m_chat->wis_r(__buf+4, __buf+28);
			break;

		case PACKET_ZC_BROADCAST:
			chatlog_check();
			m_chat->broadcast(__buf+4);
			break;

		case PACKET_ZC_CHAT_PARTY:
			chatlog_check();
			m_chat->party(__buf+8);
			break;

		case PACKET_ZC_SHOWN_MVP:
			m_chat->sys_mvp(reinterpret_cast<DWORD>(__buf+2));
			break;

#ifndef PUBLIC_RELEASE
		case PACKET_ZC_CHANGE_OPTION:
			zc_change_option(__buf);
			break;
#endif

		case PACKET_ZC_CHAT_GUILD:
			chatlog_check();
			m_chat->guild(__buf+4);
			break;

		case PACKET_ZC_TALKIE_MSG:
			chatlog_check();
			m_chat->talkie(__buf+6);
			break;

		case PACKET_ZC_CHANGE_CONDITION:
			zc_change_condition(__buf);
			break;

		case PACKET_ZC_LOCALBROADCAST:
			chatlog_check();
			m_chat->lbc(__buf+16);
			break;

#ifndef PUBLIC_RELEASE
		case PACKET_ZC_CHANGE_OPTION2:
			zc_change_option(__buf);
			break;
#endif

		default:
			break;
	}

	return __buflen;
}

////////////////////////////////////////////////////////////////////////////////


bool CPacket::isOwnAID(DWORD sid)
{
	DWORD aid_addr = conf->get_addr(ADDR_AID);
	return *reinterpret_cast<DWORD*>(aid_addr)==sid;
}


void CPacket::zc_change_option(BYTE* __buf)
{
	if (isOwnAID(rev_cast<DWORD>(__buf+2)))
	{
		WORD param1 = rev_cast<WORD>(__buf+6);
		WORD param2 = rev_cast<WORD>(__buf+8);
		WORD param3 = rev_cast<WORD>(__buf+10);

		switch (param1)
		{
			case 0x0003:
				cl_pushText("CONDITION_TO_STUN", CL_TEXT_NOTICE2);
				break;
			case 0x0004:
				cl_pushText("CONDITION_TO_CURSE_SLEEP", CL_TEXT_NOTICE2);
				break;
		}

		if (param2 & 0x0001)
			cl_pushText("CONDITION_TO_POISON", CL_TEXT_NOTICE2);
		if (param2 & 0x0010)
		{
			cl_pushText("CONDITION_TO_BLIND", CL_TEXT_NOTICE2);
			*reinterpret_cast<WORD*>(__buf+8) ^= 0x0010;
		}
	}
}


void CPacket::zc_change_condition(BYTE* __buf)
{
	if (isOwnAID(rev_cast<DWORD>(__buf+4)))
	{
		BYTE flag = rev_cast<BYTE>(__buf+8);

		switch (rev_cast<WORD>(__buf+2))
		{
#ifndef PUBLIC_RELEASE
			// NPC_HALLUCINATION
			case 0x0022:
				if (flag & 0x01)
				{
					cl_pushText(_T("CONDITION_TO_HALLUCINATION"), CL_TEXT_NOTICE2);
					*reinterpret_cast<BYTE*>(__buf+8) = 0x00;
				}
				break;
#endif

			// ITM_DARKWEAPON
			case 0x0040:
				if (flag & 0x01)
					cl_pushText(_T(" 武器に闇属性が付与されました。"), CL_TEXT_NOTICE1);
				else
					cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
				break;

			// SA_FLAMELAUNCHER
			case 0x005A:
				if (flag & 0x01)
					cl_pushText(_T(" 武器に火属性が付与されました。"), CL_TEXT_NOTICE1);
				else
					cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
				break;
			// SA_FROSTWEAPON
			case 0x005B:
				if (flag & 0x01)
					cl_pushText(_T(" 武器に水属性が付与されました。"), CL_TEXT_NOTICE1);
				else
					cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
				break;
			// SA_LIGHTNINGLOADER
			case 0x005C:
				if (flag & 0x01)
					cl_pushText(_T(" 武器に風属性が付与されました。"), CL_TEXT_NOTICE1);
				else
					cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
				break;
			// SA_SEISMICWEAPON
			case 0x005D:
				if (flag & 0x01)
					cl_pushText(_T(" 武器に地属性が付与されました。"), CL_TEXT_NOTICE1);
				else
					cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
				break;

			// LK_AURABLADE
			case 0x0067:
				if (flag & 0x01)
					cl_pushText(_T("オーラブレード状態になりました。"), CL_TEXT_NOTICE1);
				else
					cl_pushText(_T("オーラブレード状態が解除されました。"), CL_TEXT_NOTICE2);
				break;

			// HP_ASSUMPTIO
			case 0x006E:
				if (flag & 0x01)
					cl_pushText(_T("アスムプティオ状態になりました。"), CL_TEXT_NOTICE1);
				else
					cl_pushText(_T("アスムプティオ状態が解除されました。"), CL_TEXT_NOTICE2);
				break;
		}
	}
}

