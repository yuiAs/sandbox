#include "ro_parser.h"
#include "../client.h"
#include "../dbgprint.h"

extern Client* cl;

////////////////////////////////////////////////////////////////////////////////


ROPacketParser::ROPacketParser()
	: m_log(NULL), m_aid(0)
{
	init_val();
}


ROPacketParser::~ROPacketParser()
{
	destroy();
}

////////////////////////////////////////////////////////////////////////////////


void ROPacketParser::init(DWORD address)
{
	ROPacket::init(address);

	m_log = new ROLog;
	m_log->init("./Chat", "");
}


void ROPacketParser::init_val()
{
	m_elementCI = CI_NONE;
	m_truesight = 0x00;

	m_blockSTUP = false;
	m_blockSKUP = false;
	m_fixDir = false;
}


void ROPacketParser::destroy()
{
	if (m_log)
		delete m_log;
}

////////////////////////////////////////////////////////////////////////////////


int ROPacketParser::parse_r(u_char* buf, int len)
{
//	dbgprintf(5, "parse_r %04X %d\n", *reinterpret_cast<u_short*>(buf), len);

	switch (*reinterpret_cast<u_short*>(buf))
	{
		case PACKET_AC_AUTH_ACK:
			{
				m_aid = *reinterpret_cast<u_long*>(buf+8);
				dbgprintf(0, "PACKET_AC_AUTH_ACK AID=%08X\n", m_aid);
			}
			break;

		case PACKET_ZC_ENTER_ACK:
			cl_enter_ack();
			break;

		case PACKET_ZC_CHAT:
			m_log->push(buf+8, len-8, CHAT_NOR);
			break;

		case PACKET_ZC_CHAT_THIS:
			m_log->push(buf+4, len-4, CHAT_NOR);
			break;

		case PACKET_ZC_MAPCHANGE_IP:
			break;

		case PACKET_ZC_WHISPER:
			m_log->push(buf, len, CHAT_WIS_R);	// Logger側で判定するのにbufferが全部必要
			break;

		case PACKET_ZC_WHISPER_ACK:
			{
				u_char val = *reinterpret_cast<u_char*>(buf+2);
				if (val != 0x00)
					m_log->push(val, CHAT_WIS_E);
			}
			break;

		case PACKET_ZC_BROADCAST:
			m_log->push(buf+4, len-4, CHAT_GBC);
			break;

		case PACKET_ZC_SETDIR:
			if (m_fixDir)
				fix_direction(buf, 6, 8);
			break;

		case PACKET_ZC_PARTYCHAT:
			m_log->push(buf+8, len-8, CHAT_PRT);
			break;

		case PACKET_ZC_OPTION:
			cl_option(buf);
			break;

		case PACKET_ZC_GUILDCHAT:
			m_log->push(buf+4, len-4, CHAT_GLD);
			break;

		case PACKET_ZC_CONDITION:
			cl_condition(buf);
			break;

		case PACKET_ZC_BROADCAST2:
			m_log->push(buf+16, len-16, CHAT_LBC);
			break;

		case PACKET_ZC_USEITEM_ACK2:
			cl_useitem_ack(buf);
			break;

		case PACKET_ZC_OPTION2:
			cl_option(buf);
			break;

		case PACKET_ZC_ACTOR3:
			if (m_fixDir)
				fix_direction(buf, 34, 54);
			break;
	}

	return 0;
}


int ROPacketParser::parse_s(u_char* buf, int len)
{
//	dbgprintf(5, "parse_s %04X %d\n", *reinterpret_cast<u_short*>(buf), len);

	switch (*reinterpret_cast<u_short*>(buf))
	{
		case PACKET_CZ_WHISPER:
			{
				int _len = *reinterpret_cast<u_short*>(buf+2) - 4;
				m_log->push(buf+4, _len, CHAT_WIS_S);
			}
			break;

		case PACKET_CZ_STUP:
			if (local_stup())
				return len;
			break;

		case PACKET_CZ_SKUP:
			if (local_skup())
				return len;
			break;

		case PACKET_CZ_GMHIDE:
			if (local_truesight())
				return len;
			break;

		case PACKET_CZ_DEBUG:
			{
				local_debug();
				return len;
			}
			break;

		case PACKET_CA_SEED_ENQ:
			m_aid = 0;
			break;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


bool ROPacketParser::is_NumLockOFF()
{
	return ::GetKeyState(VK_NUMLOCK)==0;
}


bool ROPacketParser::is_ownAID(const u_long sid)
{
	return m_aid==sid;
}

////////////////////////////////////////////////////////////////////////////////


void ROPacketParser::push_stateNotice(u_char flag, char* target)
{
	if (flag & 0x01)
		cl->clprintf(CL_TXT_NOTICE1, "%s状態になりました。", target);
	else
		cl->clprintf(CL_TXT_NOTICE2, "%s状態が解除されました。", target);
}


void ROPacketParser::push_elementNotice(u_char flag, char* element)
{
	if (flag & 0x01)
		cl->clprintf(CL_TXT_NOTICE1, " 武器に%s属性が付与されました。", element);
	else
	{
		char* msg = NULL;
		cl->GetMsgString(MSG_RESTORE_WEAPON_ELEMENT, &msg);

		if (msg)
			cl->clprintf(CL_TXT_NOTICE2, msg);
	}
}


void ROPacketParser::push_condition(u_short type, u_char flag)
{
	u_char buf[9];
	*reinterpret_cast<u_short*>(buf) = PACKET_ZC_CONDITION;
	*reinterpret_cast<u_short*>(buf+2) = type;
	*reinterpret_cast<u_long*>(buf+4) = m_aid;
	*reinterpret_cast<u_char*>(buf+8) = flag;

	cl->SetCondition(buf);
}

////////////////////////////////////////////////////////////////////////////////


void ROPacketParser::push_elementConv(u_short type, char* element)
{
	m_elementCI = type;
	push_elementNotice(0x01, element);
	push_condition(type, 0x01);
}


void ROPacketParser::release_elementConv(u_char *buf)
{
	switch (m_elementCI)
	{
		case CI_ELEMENT_FIRE:
			push_elementNotice(0x00, "火");
			break;
		case CI_ELEMENT_WATER:
			push_elementNotice(0x00, "水");
			break;
		case CI_ELEMENT_WIND:
			push_elementNotice(0x00, "風");
			break;
		case CI_ELEMENT_GROUND:
			push_elementNotice(0x00, "地");
			break;
		case CI_ELEMENT_SHADOW:
			push_elementNotice(0x00, "暗");
			break;
		case CI_ELEMENT_GHOST:
			push_elementNotice(0x00, "念");
			break;

		default:
			return;
	}

	*reinterpret_cast<u_short*>(buf+2) = m_elementCI;
	m_elementCI = CI_NONE;
}

////////////////////////////////////////////////////////////////////////////////


void ROPacketParser::fix_direction(u_char* buf, int head, int body)
{
	if (head != -1)
	{
		if (*reinterpret_cast<u_short*>(buf+head) > 0x0002)
			*reinterpret_cast<u_short*>(buf+head) = 0x0000;
	}
	if (body != -1)
	{
		if (*reinterpret_cast<u_char*>(buf+body) > 0x07)
			*reinterpret_cast<u_char*>(buf+body) = 0x00;
	}
}

