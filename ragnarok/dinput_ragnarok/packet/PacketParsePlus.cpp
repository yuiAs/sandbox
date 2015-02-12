#include "PacketParsePlus.hpp"


////////////////////////////////////////////////////////////////////////////////


void CPacketParserPlus::initialize_plus()
{
	TCHAR* buffer = new TCHAR[1024];

	if (DWORD readsize = ::GetPrivateProfileString(_T("packet"), _T("dead_info"), _T(""), buffer, 1024, _T("./dinput.ini")))
	{
		m_flag->set(DEAD_INFO);


		typedef CStringT<TCHAR, StrTraitATL<TCHAR> > CAtlString;
		CAtlString s = buffer;
		CAtlString t;
		int pos = 0;

		t = s.Tokenize(_T(","), pos);
		while (t != "")
		{
			m_npc_dead.insert(static_cast<WORD>(_tcstoul(t, NULL, 0)));
			t = s.Tokenize(_T(","), pos);
		}
	}

	delete [] buffer;
}

////////////////////////////////////////////////////////////////////////////////


int CPacketParserPlus::parse_recv(BYTE* rwbuf, int length)
{
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

		case 0x0078:
			recv_addactor(rwbuf, length);
			break;

		case 0x0080:
			recv_eraseactor(rwbuf, length);
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

		case 0x009D:
			recv_dropitem_m(rwbuf, length);
			break;

		case 0x009E:
			recv_dropitem_s(rwbuf, length);
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
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////


int CPacketParserPlus::recv_dropitem_m(BYTE* buf, int length)
{
//	if (flag->test(THROUGH_ITEM))
//	{
//		if (p_cast<BYTE>(buf+8) == 0x00)
//			*reinterpret_cast<BYTE*>(buf+8) = 0x01;
//	}
	return 0;
}


int CPacketParserPlus::recv_dropitem_s(BYTE* buf, int length)
{
//	if (flag->test(THROUGH_ITEM))
//	{
//		if (p_cast<BYTE>(buf+8) == 0x00)
//			*reinterpret_cast<BYTE*>(buf+8) = 0x01;
//	}
	return 0;
}

int CPacketParserPlus::recv_addactor(BYTE* buf, int length)
{
	if (m_flag->test(DEAD_INFO))
	{
		DWORD dwSID = p_cast<DWORD>(buf+2);
		WORD wClass = p_cast<WORD>(buf+14);

		if (wClass > 1000)
		{
			std::set<WORD>::iterator i = m_npc_dead.find(wClass);
			if (i != m_npc_dead.end())
				m_npc.insert(m_pair_npc(dwSID, wClass));
		}
	}

	return 0;
}

int CPacketParserPlus::recv_eraseactor(BYTE* buf, int length)
{
	if (m_flag->test(DEAD_INFO))
	{
		DWORD dwSID = p_cast<DWORD>(buf+2);
		BYTE type = p_cast<BYTE>(buf+6);

		std::map<DWORD, WORD>::iterator i = m_npc.find(dwSID);
		if (i != m_npc.end())
		{
			if (type == 0x01)
			{
				TCHAR name_buf[LN_NAME_NPC];
				m_core->get_npc_id(static_cast<int>(i->second), name_buf);

				SYSTEMTIME t;
				::GetLocalTime(&t);

				TCHAR* line_buf = new TCHAR[LN_TEXT_SYSTEM];
				::wsprintf(line_buf, _T("DEAD %02d:%02d:%02d %s (%08X)"), t.wHour, t.wMinute, t.wSecond, _tcsupr(name_buf), dwSID);
				m_core->cl_pushText(line_buf, CL_TEXT_NOTICE1);
			}

			m_npc.erase(i);
		}
	}

	return 0;
}
