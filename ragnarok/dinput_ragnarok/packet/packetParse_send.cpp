#include "packetParse.hpp"


////////////////////////////////////////////////////////////////////////////////


int CPacketParser::parse_send(BYTE* rwbuf, int length)
{
	if (m_flag->test(PACKET_DUMP))
		dump(rwbuf, length, false);


	switch (p_cast<WORD>(rwbuf))
	{
		case 0x0096:
			return send_whisper(rwbuf, length);

		//case 0x00E4:
		//	return send_reqtrading(rwbuf, length);

		case 0x0108:
			return send_ptchat(rwbuf, length);

		case 0x017E:
			return send_guildchat(rwbuf, length);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


int CPacketParser::send_reqtrading(BYTE* buf, int length)
{
	if (m_flag->test(BLOCK_TRADE_REQ))
	{
		if (::GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			TCHAR* buffer = new TCHAR[64];

			_sntprintf(buffer, 64, _T("AID=%08X"), p_cast<DWORD>(buf+2));
			m_core->cl_pushText(buffer, CL_TEXT_WHISPER);

			delete [] buffer;
		}

		return length;
	}

	return 0;
}


int CPacketParser::send_whisper(BYTE* buf, int length)
{
	if (m_flag->test(CH_WHISPER))
		m_chat->output(CChatLog::CT_SENDWHISPER, buf, 4, 28);

	return 0;
}


int CPacketParser::send_ptchat(BYTE* buf, int length)
{
	if (m_flag->test(NATURAL_CHAT))
	{
		if (::GetAsyncKeyState(VK_MENU) & 0x8000)
			*reinterpret_cast<WORD*>(buf) = 0x017E;	// -> Guild Chat
	}

	return 0;
}


int CPacketParser::send_guildchat(BYTE* buf, int length)
{
	if (m_flag->test(NATURAL_CHAT))
	{
		if (::GetAsyncKeyState(VK_CONTROL) & 0x8000)
			*reinterpret_cast<WORD*>(buf) = 0x0108;	// -> Party Chat
	}

	return 0;
}
