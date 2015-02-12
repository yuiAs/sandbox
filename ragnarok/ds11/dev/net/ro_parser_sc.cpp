#include "ro_parser.h"
#include "../client.h"
#include "../dbgprint.h"

extern Client* cl;

////////////////////////////////////////////////////////////////////////////////


void ROPacketParser::cl_enter_ack()
{
	m_truesight = 0x00;

	m_blockSTUP = (::GetPrivateProfileInt(_T("parser"), _T("block_stup"), 0, _T("./ijl15.ini"))==1);
	m_blockSKUP = (::GetPrivateProfileInt(_T("parser"), _T("block_skup"), 0, _T("./ijl15.ini"))==1);
	m_fixDir = (::GetPrivateProfileInt(_T("parser"), _T("fix_direction"), 0, _T("./ijl15.ini"))==1);

	if (u_long _tmp = cl->GetAddress(AD_AID))
	{
		m_aid = *reinterpret_cast<u_long*>(_tmp);
		dbgprintf(0, "PACKET_ZC_ENTER_ACK AID=%08X\n", m_aid);
	}
}

////////////////////////////////////////////////////////////////////////////////


void ROPacketParser::cl_option(u_char* buf)
{
	if (is_ownAID(*reinterpret_cast<u_long*>(buf+2)))
		cl_option_own(buf);
}


void ROPacketParser::cl_option_own(u_char* buf)
{
	u_short p1 = *reinterpret_cast<u_short*>(buf+6);
	u_short p2 = *reinterpret_cast<u_short*>(buf+8);
	u_short p3 = *reinterpret_cast<u_short*>(buf+10);
	dbgprintf(0, "PACKET_ZC_OPTION p1=%04X p2=%04X p3=%04X\n", p1, p2, p3);

	switch (p1)
	{
		case 0x0003:
			cl->clprintf(CL_TXT_NOTICE2, "CONDITION_TO_STUN");
			break;
		case 0x0004:
			cl->clprintf(CL_TXT_NOTICE2, "CONDITION_TO_CURSE_SLEEP");
			break;
	}

	if (p2 & 0x0001)
		cl->clprintf(CL_TXT_NOTICE2, "CONDITION_TO_POISON");
	if (p2 & 0x0010)
	{
		cl->clprintf(CL_TXT_NOTICE2, "CONDITION_TO_BLIND");
		*reinterpret_cast<u_short*>(buf+8) ^= 0x0010;
	}
}

////////////////////////////////////////////////////////////////////////////////


void ROPacketParser::cl_condition(u_char* buf)
{
	if (is_ownAID(*reinterpret_cast<u_long*>(buf+4)) == false)
		return;


	u_short type = *reinterpret_cast<u_short*>(buf+2);
	u_char flag = *reinterpret_cast<u_char*>(buf+8);
	dbgprintf(0, "PACKET_ZC_CONDITION type=%04X flag=%02X\n", type, flag);

	switch (type)
	{
		case CI_HALLUCINATION:
			if (flag & 0x01)
			{
				cl->clprintf(CL_TXT_NOTICE2, "CONDITION_TO_HULLCINATION");
				*reinterpret_cast<u_char*>(buf+8) = 0x00;
			}
			break;

		case CI_ELEMENTALCONV:
			if (flag == 0x00)
				release_elementConv(buf);
			break;

		case CI_ELEMENT_FIRE:
			push_elementNotice(flag, "火");
			break;
		case CI_ELEMENT_WATER:
			push_elementNotice(flag, "水");
			break;
		case CI_ELEMENT_WIND:
			push_elementNotice(flag, "風");
			break;
		case CI_ELEMENT_GROUND:
			push_elementNotice(flag, "地");
			break;

		case CI_BERSERK:
			push_stateNotice(flag, "バーサーク");
			break;

		case CI_ASSUMPTIO:
			push_stateNotice(flag, "アスムプティオ");
			break;

		case CI_ELEMENT_SHADOW:
			push_elementNotice(flag, "暗");
			break;
		case CI_ELEMENT_GHOST:
			push_elementNotice(flag, "念");
			break;

		case CI_CLOSECONFINE_OWN:
		case CI_CLOSECONFINE_TGT:
			push_stateNotice(flag, "クローズコンファイン");
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////


void ROPacketParser::cl_useitem_ack(u_char *buf)
{
	u_short iid = *reinterpret_cast<u_short*>(buf+4);
	u_long sid = *reinterpret_cast<u_long*>(buf+6);
	u_char flag = *reinterpret_cast<u_char*>(buf+12);

	if (is_ownAID(sid) == false)
		return;
	if (flag != 0x01)
		return;

	// 12020#呪われた水#
	// 12114#火属性コンバーター#
	// 12115#水属性コンバーター#
	// 12116#地属性コンバーター#
	// 12117#風属性コンバーター#

	switch (iid)
	{
		case 12020:
			push_elementConv(CI_ELEMENT_SHADOW, "闇");
			break;
		case 12114:
			push_elementConv(CI_ELEMENT_FIRE, "火");
			break;
		case 12115:
			push_elementConv(CI_ELEMENT_WATER, "水");
			break;
		case 12116:
			push_elementConv(CI_ELEMENT_GROUND, "地");
			break;
		case 12117:
			push_elementConv(CI_ELEMENT_WIND, "風");
			break;

		default:
			break;
	}
}