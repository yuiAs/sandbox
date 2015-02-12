#include "../common.h"
#include "packet.hpp"
#include "define.h"
#include "../patch/client.hpp"

////////////////////////////////////////////////////////////////////////////////


int CPacket::parse_r(u_char* buf, int len)
{
	switch (_mkU16(buf))
	{
#ifdef _DEBUG
		case PACKET_AC_AUTH_SUCCEEDED:
			dbgprintf(0, "AC_AUTH_SUCCEEDED aid=%08X\n", _mkU32(buf+8));
			break;
#endif

#ifdef _EXTENTION
		case PACKET_ZC_CONNECT_SUCCEEDED:
			m_truesight = 0x00;
			break;
#endif

		case PACKET_ZC_CHAT_NORMAL:
			if (m_chat)
				m_chat->push(buf, len, ChatLog::CHAT_NORMAL);
			break;

		case PACKET_ZC_CHAT_NORMAL_OWN:
			if (m_chat)
				m_chat->push(buf, len, ChatLog::CHAT_NORMAL_OWN);
			break;

#ifdef _EXTENTION
		case PACKET_ZC_CHANGE_MAP_WITH_ZONE:
			{
				m_truesight = 0x00;

				if (m_currentCI != CI_NONE)
					push_changeCondition(m_currentCI, 0x01);
			}
			break;
#endif

		case PACKET_ZC_WHISPER:
			if (m_chat)
				m_chat->push(buf, len, ChatLog::CHAT_WIS_R);
			break;

		case PACKET_ZC_BROADCAST:
			if (m_chat)
				m_chat->push(buf, len, ChatLog::CHAT_BC);
			break;

#ifdef _EXTENTION
		case PACKET_ZC_CHANGEDIR:
			cl_changedir_fix(buf);
			break;
#endif

		case PACKET_ZC_CHAT_PARTY:
			if (m_chat)
				m_chat->push(buf, len, ChatLog::CHAT_PARTY);
			break;

		case PACKET_ZC_SHOWN_MVP:
			if (m_chat)
				m_chat->push(_mkU32(buf+2), ChatLog::SYS_MVP);
			break;

#ifdef _EXTENTION
		case PACKET_ZC_CHANGE_OPTION:
			cl_change_option(buf);
			break;
#endif

		case PACKET_ZC_CHAT_GUILD:
			if (m_chat)
				m_chat->push(buf, len, ChatLog::CHAT_GUILD);
			break;

		case PACKET_ZC_CHANGE_CONDITION:
			cl_change_condition(buf);
			break;

		case PACKET_ZC_LOCALBROADCAST:
			if (m_chat)
				m_chat->push(buf, len, ChatLog::CHAT_LBC);
			break;

		case PACKET_ZC_NOTICE_USEITEM:
			cl_notice_useitem(buf);
			break;

#ifdef _EXTENTION
		case PACKET_ZC_CHANGE_OPTION_NEW:
			cl_change_option(buf);
			break;

		case PACKET_ZC_ACTORINFO_1:
			cl_changedir_fix(buf);
			break;
#endif
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


int CPacket::parse_s(u_char* buf, int len)
{
	switch (*reinterpret_cast<u_short*>(buf))
	{
#ifdef _DEBUG
		case PACKET_CZ_LOADING:
			dbgprintf(0, "charname=%s\n", reinterpret_cast<char*>(client::address(client::AD_NAME)));
			break;
#endif

		case PACKET_CZ_WHISPER:
			if (m_chat)
				m_chat->push(buf, len, ChatLog::CHAT_WIS_S);
			break;

#ifdef _EXTENTION

		case PACKET_CZ_STATUSUP_REQ:
			if (m_blockSTUp)
			{
				if (local_0x00BB())
					return len;
			}
			break;

		case PACKET_CZ_REQ_ACTORACOUNT:
			{
				local_debugMsg();
				return len;
			}
			break;

		case PACKET_CZ_SKILLUP_REQ:
			if (m_blockSKUp)
			{
				if (local_0x0112())
					return len;
			}
			break;

		case PACKET_CZ_GM_HIDE:
			{
				if (local_trueSight())
					return len;
			}
			break;

#endif

	}

	return 0;
}
