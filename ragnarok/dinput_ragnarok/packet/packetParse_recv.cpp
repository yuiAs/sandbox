#include "packetParse.hpp"


////////////////////////////////////////////////////////////////////////////////


int CPacketParser::parse_recv(BYTE* rwbuf, int length)
{
	if (m_flag->test(PACKET_DUMP))
		dump(rwbuf, length, true);


	switch (p_cast<WORD>(rwbuf))
	{
		case 0x0069:
			recv_svinfo(rwbuf, length);
			break;

		case 0x006B:
			recv_charinfo(rwbuf, length);
			break;

		case 0x006D:
			recv_charmakingsucceeded(rwbuf, length);
			break;

		case 0x0071:
			recv_zoneinfo(rwbuf, length);
			break;

		case 0x0073:
			recv_connectzone(rwbuf, length);
			break;

		case 0x008D:
			recv_normalchat(rwbuf, length);
			break;

		case 0x008E:
			recv_normalchat_own(rwbuf, length);
			break;

		case 0x0091:
			recv_changemap(rwbuf, length);
			break;

		case 0x0092:
			recv_changezone(rwbuf, length);
			break;

		case 0x0097:
			recv_whisper(rwbuf, length);
			break;

		case 0x009A:
			recv_broadcast(rwbuf, length);
			break;

		case 0x00B0:
			recv_updatestatus_1(rwbuf, length);
			break;

		case 0x00B1:
			recv_updatestatus_2(rwbuf, length);
			break;

		case 0x0109:
			recv_ptchat(rwbuf, length);
			break;

		case 0x010C:
			recv_mvp(rwbuf, length);
			break;

		case 0x0119:
			recv_changeoption(rwbuf, length);
			break;

		case 0x017F:
			recv_guildchat(rwbuf, length);
			break;

		case 0x0191:
			recv_talkie(rwbuf, length);
			break;

		case 0x0196:
			recv_changecondition(rwbuf, length);
			break;

		case 0x1C3:
			recv_localbroadcast(rwbuf, length);
			break;

		case 0x0229:
			recv_changeoption(rwbuf, length);
			break;

		default:			
			break;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


int CPacketParser::recv_svinfo(BYTE* buf, int length)
{
	m_core->output(_T("AID=%08X"), p_cast<DWORD>(buf+8));
	m_core->setActorVal(ACTOR_SID, p_cast<DWORD>(buf+8));

	return 0;
}

int CPacketParser::recv_charinfo(BYTE* buf, int length)
{
	// 初期化

	m_core->setActorVal(ACTOR_CID, 0);
	m_core->setCharIP(m_currentIP);
	m_core->resetEvent(SL_ZONE);
	m_core->setEvent(SL_CHAR);					// これは初期化最後に。

	::ZeroMemory(m_currentMap, LN_NAME_MAP);
	m_cs.clear();
	
	// get character info
    
	int len = p_cast<WORD>(buf+2) - 4;
	int pad = 0;
    
	if (len % CHARDATA_LENGTH)
	{
		pad = KRO_CS_PADDING;
		len -= pad;
	}
    
	for (int i=0; i<len; i+=CHARDATA_LENGTH)
	{
		ACTOR_DATA data;
		::ZeroMemory(&data, sizeof(ACTOR_DATA));

		data.value[ACTOR_CID] = p_cast<DWORD>(buf+4+pad+i);
		data.value[ACTOR_BASEEXP] = p_cast<DWORD>(buf+4+pad+4+i);
		data.value[ACTOR_ZENY] = p_cast<DWORD>(buf+4+pad+8+i);
		data.value[ACTOR_JOBEXP] = p_cast<DWORD>(buf+4+pad+12+i);
		data.value[ACTOR_CLASS] = p_cast<WORD>(buf+4+pad+52+i);
		::CopyMemory(data.name, buf+4+pad+74+i, LN_NAME_CHAR);

		m_cs.insert(m_pair_cs(data.value[ACTOR_CID], data));
	}

	return 0;
}

int CPacketParser::recv_charmakingsucceeded(BYTE* buf, int length)
{
	ACTOR_DATA data;
	::ZeroMemory(&data, sizeof(ACTOR_DATA));

	data.value[ACTOR_CID] = p_cast<DWORD>(buf+2);
	data.value[ACTOR_BASEEXP] = p_cast<DWORD>(buf+2+4);
	data.value[ACTOR_ZENY] = p_cast<DWORD>(buf+2+8);
	data.value[ACTOR_JOBEXP] = p_cast<DWORD>(buf+2+12);
	data.value[ACTOR_CLASS] = p_cast<WORD>(buf+2+52);
	//data.value[ACTOR_BASELV] = p_cast<WORD>(buf+2+58);
	::CopyMemory(data.name, buf+2+74, LN_NAME_CHAR);

	m_cs.insert(m_pair_cs(data.value[ACTOR_CID], data));

	return 0;
}

int CPacketParser::recv_zoneinfo(BYTE* buf, int length)
{
	DWORD dwCID = p_cast<DWORD>(buf+2);
	m_core->setActorVal(ACTOR_CID, dwCID);
	m_core->output(_T("0x%04X_CID=%08X"), p_cast<WORD>(buf), dwCID);

	std::map<DWORD, ACTOR_DATA>::iterator i = m_cs.find(dwCID);
	if (i != m_cs.end())
	{
		m_core->setActorVal(ACTOR_BASEEXP, i->second.value[ACTOR_BASEEXP]);
		m_core->setActorVal(ACTOR_JOBEXP, i->second.value[ACTOR_JOBEXP]);
		m_core->setActorName(i->second.name);


		::CopyMemory(m_currentMap, buf+6, LN_NAME_MAP);

		if (m_flag->test(CH_SYSTEM))
			m_chat->output(CChatLog::CT_ZONEINFO, buf, 6, i->second.name);
	}

	m_cs.clear();

	return 0;
}

int CPacketParser::recv_connectzone(BYTE* buf, int length)
{
	m_packetBuf_length = 0;

	m_core->resetEvent(SL_CHAR);
	m_core->setEvent(SL_ZONE);

	return 0;
}

int CPacketParser::recv_changemap(BYTE* buf, int length)
{
	m_packetBuf_length = 0;

	if (::lstrcmpi(m_currentMap, reinterpret_cast<const TCHAR*>(buf+2)))
	{
		::CopyMemory(m_currentMap, buf+2, LN_NAME_MAP);

		if (m_flag->test(CH_SYSTEM))
			m_chat->output(CChatLog::CT_CHANGEMAP, buf, 2);
	}

	return 0;
}

int CPacketParser::recv_changezone(BYTE* buf, int length)
{
	m_packetBuf_length = 0;

	if (m_flag->test(CH_SYSTEM))
		m_chat->output(CChatLog::CT_CHANGEZONE, buf, 2);

	return 0;
}

int CPacketParser::recv_normalchat(BYTE* buf, int length)
{
	if (m_flag->test(CH_NORMAL))
		m_chat->output(CChatLog::CT_NORMALCHAT, buf, 8);

	return 0;
}

int CPacketParser::recv_normalchat_own(BYTE* buf, int length)
{
	if (m_flag->test(CH_NORMAL))
		m_chat->output(CChatLog::CT_NORMALCHAT_OWN, buf, 4);

	return 0;
}

int CPacketParser::recv_whisper(BYTE* buf, int length)
{
	if (m_flag->test(CH_WHISPER))
	{
		// [op02][len02][name24] 28 29 30 31 [body??]
		// 0x00FFFFFF以上のAIDは存在しないと仮定
		if (p_cast<BYTE>(buf+31) != 0x00)
			m_chat->output(CChatLog::CT_RECVWHISPER, buf, 4, 28);
		else
			m_chat->output(CChatLog::CT_RECVWHISPER, buf, 4, 28+4);
	}

	return 0;
}

int CPacketParser::recv_broadcast(BYTE* buf, int length)
{
	if (m_flag->test(CH_BROADCAST))
		m_chat->output(CChatLog::CT_BROADCAST, buf, 4);

	return 0;
}

int CPacketParser::recv_ptchat(BYTE* buf, int length)
{
	if (m_flag->test(CH_PARTY))
		m_chat->output(CChatLog::CT_PTCHAT, buf, 8);

	return 0;
}

int CPacketParser::recv_guildchat(BYTE* buf, int length)
{
	if (m_flag->test(CH_GUILD))
		m_chat->output(CChatLog::CT_GUILDCHAT, buf, 4);

	return 0;
}

int CPacketParser::recv_talkie(BYTE* buf, int length)
{
	if (m_flag->test(CH_TALKIE))
		m_chat->output(CChatLog::CT_TALKIE, buf, 6);

	return 0;
}

int CPacketParser::recv_localbroadcast(BYTE* buf, int length)
{
	if (m_flag->test(CH_LOCALBC))
		m_chat->output(CChatLog::CT_LOCALBROADCAST, buf, 16);

	return 0;
}

int CPacketParser::recv_mvp(BYTE* buf, int length)
{
	if (m_flag->test(CH_SYSTEM))
		m_chat->output(CChatLog::CT_MVP, buf, p_cast<DWORD>(buf+2));

	return 0;
}

int CPacketParser::recv_updatestatus_1(BYTE* buf, int length)
{
	DWORD value = p_cast<DWORD>(buf+4);

	switch (p_cast<WORD>(buf+2))
	{
		case 0x0005:
			m_core->setActorVal(ACTOR_CURHP, value);
			break;
		case 0x0006:
			m_core->setActorVal(ACTOR_MAXHP, value);
			break;
		case 0x0007:
			m_core->setActorVal(ACTOR_CURSP, value);
			break;
		case 0x0008:
			m_core->setActorVal(ACTOR_MAXSP, value);
			break;
		case 0x0018:
			// Weight
			break;
		case 0x0019:
			// WeightMax
			break;
	}

	return 0;
}


int CPacketParser::recv_updatestatus_2(BYTE* buf, int length)
{
	DWORD value = p_cast<DWORD>(buf+4);

	switch (p_cast<WORD>(buf+2))
	{
		case 0x0001:
			m_core->setActorVal(ACTOR_BASEEXP, value);
			break;
		case 0x0002:
			m_core->setActorVal(ACTOR_JOBEXP, value);
			break;
		case 0x0014:
			m_core->setActorVal(ACTOR_ZENY, value);
			break;
		case 0x0016:
			m_core->setActorVal(ACTOR_BASENEXT, value);
			break;
		case 0x0017:
			m_core->setActorVal(ACTOR_JOBNEXT, value);
			break;
	}

	return 0;
}

int CPacketParser::recv_changeoption(BYTE* buf, int length)
{
	if (m_flag->test(EFFECTIVE_MSG))
	{
		DWORD dwSID = p_cast<DWORD>(buf+2);

		if (dwSID == m_core->getActorVal(ACTOR_SID))
		{
			WORD param1 = p_cast<WORD>(buf+6);
			WORD param2 = p_cast<WORD>(buf+8);
			WORD param3 = p_cast<WORD>(buf+10);

			switch (param1)
			{
				case 0x0003:
					m_core->cl_pushText(_T("CONDITION_TO_STUN"), CL_TEXT_NOTICE2);
					break;
				case 0x0004:
					m_core->cl_pushText(_T("CONDITION_TO_CURSE_SLEEP"), CL_TEXT_NOTICE2);
					break;
			}

			if (param2 & 0x0001)
				m_core->cl_pushText(_T("CONDITION_TO_POISON"), CL_TEXT_NOTICE2);
			if (param2 & 0x0010)
			{
				m_core->cl_pushText(_T("CONDITION_TO_BLIND"), CL_TEXT_NOTICE2);
				*reinterpret_cast<WORD*>(buf+8) ^= 0x0010;
			}
		}
	}

	return 0;
}

int CPacketParser::recv_changecondition(BYTE* buf, int length)
{
	bool effective = m_flag->test(EFFECTIVE_MSG);
	bool advanced = m_flag->test(ADVANCED_MSG);

	if (effective || advanced)
	{
		DWORD dwSID = p_cast<DWORD>(buf+4);

		if (dwSID == m_core->getActorVal(ACTOR_SID))
		{
			BYTE flag = p_cast<BYTE>(buf+8);

			switch (p_cast<WORD>(buf+2))
			{
				// NPC_HALLUCINATION
				case 0x0022:
					if (effective && (flag & 0x01))
					{
						m_core->cl_pushText(_T("CONDITION_TO_HALLUCINATION"), CL_TEXT_NOTICE2);
						*reinterpret_cast<BYTE*>(buf+8) = 0x00;
					}
					break;

				// ITM_DARKWEAPON
				case 0x0040:
					if (flag & 0x01)
						m_core->cl_pushText(_T(" 武器に闇属性が付与されました。"), CL_TEXT_NOTICE1);
					else
						m_core->cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
					break;

				// SA_FLAMELAUNCHER
				case 0x005A:
					if (flag & 0x01)
						m_core->cl_pushText(_T(" 武器に火属性が付与されました。"), CL_TEXT_NOTICE1);
					else
						m_core->cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
					break;
				// SA_FROSTWEAPON
				case 0x005B:
					if (flag & 0x01)
						m_core->cl_pushText(_T(" 武器に水属性が付与されました。"), CL_TEXT_NOTICE1);
					else
						m_core->cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
					break;
				// SA_LIGHTNINGLOADER
				case 0x005C:
					if (flag & 0x01)
						m_core->cl_pushText(_T(" 武器に風属性が付与されました。"), CL_TEXT_NOTICE1);
					else
						m_core->cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
					break;
				// SA_SEISMICWEAPON
				case 0x005D:
					if (flag & 0x01)
						m_core->cl_pushText(_T(" 武器に地属性が付与されました。"), CL_TEXT_NOTICE1);
					else
						m_core->cl_pushText(_T(" 武器の属性が元に戻りました。"), CL_TEXT_NOTICE2);
					break;

				// LK_AURABLADE
				case 0x0067:
					if (flag & 0x01)
						m_core->cl_pushText(_T("オーラブレード状態になりました。"), CL_TEXT_NOTICE1);
					else
						m_core->cl_pushText(_T("オーラブレード状態が解除されました。"), CL_TEXT_NOTICE2);
					break;

				// HP_ASSUMPTIO
				case 0x006E:
					if (flag & 0x01)
						m_core->cl_pushText(_T("アスムプティオ状態になりました。"), CL_TEXT_NOTICE1);
					else
						m_core->cl_pushText(_T("アスムプティオ状態が解除されました。"), CL_TEXT_NOTICE2);
					break;
			}
		}
	}

	return 0;
}


