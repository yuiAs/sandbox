#include "ro_parser.h"
#include "../client.h"
#include "../dbgprint.h"

extern Client* cl;

////////////////////////////////////////////////////////////////////////////////


bool ROPacketParser::local_truesight()
{
	if (is_NumLockOFF())
		return false;

	m_truesight ^= 0x01;

	push_stateNotice(m_truesight, "TrueSight");
	push_condition(CI_TRUESIGHT, m_truesight);

	return true;
}

////////////////////////////////////////////////////////////////////////////////


bool ROPacketParser::local_stup()
{
	if (m_blockSTUP == false)
		return false;
	if (is_NumLockOFF())
		return false;

	cl->clprintf(CL_TXT_NOTICE5, "BLOCKED PACKET_CZ_STUP");
	return true;
}


bool ROPacketParser::local_skup()
{
	if (m_blockSKUP == false)
		return false;
	if (is_NumLockOFF())
		return false;

	cl->clprintf(CL_TXT_NOTICE5, "BLOCKED PACKET_CZ_SKUP");
	return true;
}

////////////////////////////////////////////////////////////////////////////////


void ROPacketParser::local_debug()
{
	u_long memAID = *reinterpret_cast<u_long*>(cl->GetAddress(AD_AID));
//	cl->clprintf(CL_TXT_NOTICE5, "[DEBUG] PLT=%08X", cl->GetAddress(AD_PLT));
	cl->clprintf(CL_TXT_NOTICE5, "[DEBUG] AID CUR=%08X MEM=%08X", m_aid, memAID);
	cl->clprintf(CL_TXT_NOTICE5, "[DEBUG] TRUESIGHT=%d", m_truesight);
	cl->clprintf(CL_TXT_NOTICE5, "[DEBUG] FIX_DIRECTION=%d BLOCK_STUP=%d BLOCK_SKUP=%d", m_fixDir, m_blockSTUP, m_blockSKUP);
}
