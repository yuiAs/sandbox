#include "packetParse.hpp"
#include "packetTable.hpp"

////////////////////////////////////////////////////////////////////////////////


CPacketParser::CPacketParser()
{
	extern CRagnarok* core;
	m_core = core;
	m_chat = 0;
	m_plog = 0;

	m_flag = new std::bitset<PACKET_FLAG_END>;

	m_packetBuf = new BYTE[MAX_PACKETBUFFER_SIZE];
	m_packetBuf_length = 0;
	m_table = 0;
	m_lastOp = 0;

	m_currentMap = new TCHAR[LN_NAME_MAP];
	m_currentIP = 0;

	m_cs.clear();


	initialize();
}


CPacketParser::~CPacketParser()
{
	delete m_flag;

	delete [] m_currentMap;
	delete [] m_table;
	delete [] m_packetBuf;

	delete m_plog;
	delete m_chat;
}

////////////////////////////////////////////////////////////////////////////////


void CPacketParser::initialize()
{
	if (::GetPrivateProfileInt(_T("log"), _T("system"), 0, _T("./dinput.ini")))
		m_flag->set(CH_SYSTEM);
	if (::GetPrivateProfileInt(_T("log"), _T("normal"), 0, _T("./dinput.ini")))
		m_flag->set(CH_NORMAL);
	if (::GetPrivateProfileInt(_T("log"), _T("broadcast"), 0, _T("./dinput.ini")))
		m_flag->set(CH_BROADCAST);
	if (::GetPrivateProfileInt(_T("log"), _T("localbroadcast"), 0, _T("./dinput.ini")))
		m_flag->set(CH_LOCALBC);
	if (::GetPrivateProfileInt(_T("log"), _T("party"), 0, _T("./dinput.ini")))
		m_flag->set(CH_PARTY);
	if (::GetPrivateProfileInt(_T("log"), _T("guild"), 0, _T("./dinput.ini")))
		m_flag->set(CH_GUILD);
	if (::GetPrivateProfileInt(_T("log"), _T("whisper"), 0, _T("./dinput.ini")))
		m_flag->set(CH_WHISPER);
	if (::GetPrivateProfileInt(_T("log"), _T("talkie"), 0, _T("./dinput.ini")))
		m_flag->set(CH_TALKIE);

	if (m_flag->any())
	{
		::CreateDirectory(_T("Chat"), NULL);
		m_chat = new CChatLog;
	}

	if (::GetPrivateProfileInt(_T("system"), _T("natural_chat"), 0, _T("./dinput.ini")))
		m_flag->set(NATURAL_CHAT);
	if (::GetPrivateProfileInt(_T("system"), _T("advanced_msg"), 0, _T("./dinput.ini")))
		m_flag->set(ADVANCED_MSG);

	if (::GetPrivateProfileInt(_T("packet"), _T("effective_msg"), 0, _T("./dinput.ini")))
		m_flag->set(EFFECTIVE_MSG);
	if (::GetPrivateProfileInt(_T("packet"), _T("block_require_trade"), 0, _T("./dinput.ini")))
		m_flag->set(BLOCK_TRADE_REQ);


	if (::GetPrivateProfileInt(_T("packet"), _T("dump"), 0, _T("./dinput.ini")))
	{
		m_plog = new COutputFile;
		if (m_plog != 0)
		{
			TCHAR filename[MAX_PATH];
			_sntprintf(filename, MAX_PATH, _T("_PACKET_%08X.log"), ::GetTickCount());
			m_plog->acquire(filename);

			m_flag->set(PACKET_DUMP);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


void CPacketParser::buildTable(DWORD address, int pos)
{
	CPacketTable* tbl = new CPacketTable;
	tbl->build(address, pos);

	int size = tbl->size();
	m_lastOp = size - 1;
	m_table = new int[size];
	::CopyMemory(m_table, tbl->get(), sizeof(int)*size);

	delete tbl;
}

////////////////////////////////////////////////////////////////////////////////


int CPacketParser::parseRecv(BYTE* buf, int length)
{
	if (m_table == 0)
		return 0;

	// AIDだけが0x006Bの前に送られてくる場合がある

	if ((length == 4) && (p_cast<DWORD>(buf) == m_core->getActorVal(ACTOR_SID)))
		return 0;


	BYTE* p = buf;
	int rest = length;
	int current = 0;

	if (m_packetBuf_length > 0)
	{
		// 未処理のbufferが存在する場合

		// m_packetBuf_length=1の場合はop判定時にbufから1byte持ってくる必要がある
		// 同様にm_packetBuf_length<4でパケット長がパケット内に含まれる場合はbufを見ないといけない

		// 先にバッファにコピー
		// 実際bufferoverflowになることはありえないと思うのでとりあえず単純コピー
		::CopyMemory(m_packetBuf+m_packetBuf_length, p, length);

		// op/packet_lengthチェック

		int n = this->length(p_cast<WORD>(m_packetBuf));
		if (n == -1)
			n = p_cast<WORD>(m_packetBuf+2);

		int j = n - (m_packetBuf_length + length);
		if (j > 0)
		{
			// まだ足りないならさらにskip
			m_packetBuf_length += length;
			return 0;
		}
		else
		{
			int r = n - m_packetBuf_length;			// buf側にある未処理bufferの残り長さ

			BYTE* lp = new BYTE[n];

			::CopyMemory(lp, m_packetBuf, m_packetBuf_length);
			::CopyMemory(lp+m_packetBuf_length, buf, r);
			parse_recv(lp, n);
			
			delete [] lp;

			current += r;
			rest -= r;
		}
	}

	while (rest >= 2)		// 2以上でないとopの判定が出来ない
	{
		int n = this->length(p_cast<WORD>(p+current));	// 処理op長
		if (n == -1)
		{
			if (rest >= 4)	// 4以上でないとパケット長の判定が出来ない
				n = p_cast<WORD>(p+current+2);
			else
				break;
		}

		if (n > rest)
			break;		// 未処理分よりop長が大きい場合は次のpacketと結合

		if (n != 1)		// 処理可能op
			parse_recv(p+current, n);

		current += n;
		rest -= n;
	}

	if (rest > 0)		// 未処理Bufferがある場合(restが負数は考慮しない)
	{
		::ZeroMemory(m_packetBuf, MAX_PACKETBUFFER_SIZE);
		::CopyMemory(m_packetBuf, p+current, rest);
		m_packetBuf_length = rest;
	}
	else
		m_packetBuf_length = 0;


	return 0;
}


int CPacketParser::parseSend(BYTE* buf, int length)
{
	if (m_table == 0)
		return 0;

	int n = this->length(p_cast<WORD>(buf));	// 処理op長
	if (n == -1)
		n = p_cast<WORD>(buf+2);

	return parse_send(buf, n);
}

////////////////////////////////////////////////////////////////////////////////


void CPacketParser::dump(BYTE* buf, int length, bool recv)
{
	if ((m_plog == 0) || (buf == 0))
		return;

	int bufsize = length*3 + 64;

	TCHAR* log = new TCHAR[bufsize];
	TCHAR* p = log;

	_sntprintf(p, bufsize, _T("[%08X] [%s] "), ::GetTickCount(), recv?_T("R"):_T("S"));
	p += ::lstrlen(p);

	for (int i=0; i<length; i++)
	{
		_sntprintf(p, bufsize, _T("%02X "), buf[i]);
		p += 3;
	}

	_sntprintf(p, bufsize, _T("\r\n"));
	m_plog->write(log, ::lstrlen(log));

	delete [] log;
}

