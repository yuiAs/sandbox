#include "ro_packet.h"
#include "../dbgprint.h"

////////////////////////////////////////////////////////////////////////////////


ROPacket::ROPacket()
	: m_recv(NULL), m_send(NULL), m_plt(0)
{
}


ROPacket::~ROPacket()
{
	clear_recvque();
	clear_sendque();
}

////////////////////////////////////////////////////////////////////////////////


void ROPacket::init(DWORD address)
{
	search_plt(address);

	if (m_plt == 0)
	{
		dbgprintf(0, "failed plt_auto_search\n");
		return;
	}

	{
		NODE* root = reinterpret_cast<NODE*>(m_plt);
		NODE* st = root->left;
		NODE* ed = root->right;
		dbgprintf(0, "root=%08X rootpar=%08X st=%08X ed=%08X stop=%04X edop=%04X\n",
						m_plt, reinterpret_cast<NODE*>(m_plt)->parent, st, ed, st->op, ed->op);

//		for (u_long i=st->op; i!=ed->op; i++)
//			dbgprintf(2, "0x%04X=%d\n", i, search_pltnode(root->parent, static_cast<u_short>(i)));
	}

	m_recv = ::GetProcAddress(::GetModuleHandle(_T("ws2_32.dll")), "recv");
	m_send = ::GetProcAddress(::GetModuleHandle(_T("ws2_32.dll")), "send");

	clear_recvque();
	clear_sendque();
}


void ROPacket::init_api(void* recv, void* send)
{
	m_recv = recv;
	m_send = send;
}

////////////////////////////////////////////////////////////////////////////////


void ROPacket::search_plt(DWORD address)
{
	for (DWORD i=address; i<address+SEARCH_RANGE; i+=4)
	{
		__try
		{
			DWORD cur = *reinterpret_cast<DWORD*>(i);
			NODE* tmp = reinterpret_cast<NODE*>(cur);

			if (tmp == NULL)
				continue;
			if (tmp->color != 0)
				continue;
			if (tmp->left == NULL)
				continue;

			if (tmp->left->color == 1)
			{
				m_plt = cur;
				dbgprintf(0, "plt=%08X plt_top=%08X root=%08X\n", i, i-4, m_plt);
				break;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			continue;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


int ROPacket::get_length(NODE* node)
{
	if (node->length == PLT_UNDEFINED)
		return PLT_DEFAULT;
	else
		return node->length;
}


int ROPacket::get_length(u_char* buf)
{
	if (m_plt)
	{
		u_short op = *reinterpret_cast<u_short*>(buf);

		// 例外処理

		if (op == 0x0069)
		{
			// 0x0069の直後に単発でAIDが来るため
			// 0x0069のパケット長を4byte増やしてParserに無視させる
			int length = *reinterpret_cast<u_short*>(buf+2) + 4;
			return length;
		}

		int length = search_pltnode(reinterpret_cast<NODE*>(m_plt)->parent, op);
		dbgprintf(0, "op=%04X len=%d\n", op, length);

		if (length != PLT_UNDEFINED)
			return length;
	}

	return PLT_DEFAULT;
}

////////////////////////////////////////////////////////////////////////////////


int ROPacket::search_pltnode(NODE* node, u_short op)
{
	if (node == NULL)
		return PLT_UNDEFINED;

	const u_short curOp = static_cast<u_short>(node->op);

	if (curOp == op)
		return node->length;
	else
	{
		if (curOp > op)
			return search_pltnode(node->left, op);
		if (curOp < op)
			return search_pltnode(node->right, op);

		return PLT_UNDEFINED;
	}
}

////////////////////////////////////////////////////////////////////////////////


void ROPacket::clear_recvque()
{
	while (m_recvQue.size())
	{
		PQUE tmp = m_recvQue.front();
		m_recvQue.pop();
		
		if (tmp.buf)
			delete [] tmp.buf;
	}
}


void ROPacket::recv_f(u_char* buf, int buflen)
{
	PQUE tmp = { buflen, buf };
	m_recvQue.push(tmp);
}


int ROPacket::exec_recvque(char* buf, int rest)
{
	if (rest < 0)
		return 0;

	int result = 0;

	while (m_recvQue.size())
	{
		PQUE tmp = m_recvQue.front();

		if ((result+tmp.len) > rest)
			break;
		else
		{
			m_recvQue.pop();

			if (tmp.buf)
			{
				memcpy(buf+result, tmp.buf, tmp.len);
				delete [] tmp.buf;
				result += tmp.len;
			}
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////


void ROPacket::clear_sendque()
{
	while (m_sendQue.size())
	{
		PQUE tmp = m_sendQue.front();
		m_sendQue.pop();

		if (tmp.buf)
			delete [] tmp.buf;
	}

	m_sendQueLen = 0;
}


void ROPacket::send_f(u_char* buf, int buflen)
{
	PQUE tmp = { buflen, buf };
	m_sendQue.push(tmp);

	m_sendQueLen += buflen;
}


int ROPacket::exec_sendque(SOCKET s, char* buf, int len, int flags)
{
	if (m_sendQueLen == 0)
		return 0;

	int execPos = 0;
	int execLen = ((m_sendQueLen+len)>2048) ? 2048 : (m_sendQueLen+len);

	char* execBuf = new char[execLen];
	memcpy(execBuf, buf, len);
	execPos += len;

	while (m_sendQue.size())
	{
		PQUE tmp = m_sendQue.front();

		if ((execPos+tmp.len) > execLen)
			break;
		else
		{
			m_sendQue.pop();

			if (tmp.buf)
			{
				memcpy(execBuf+execPos, tmp.buf, tmp.len);
				delete [] tmp.buf;
				execPos += tmp.len;

				m_sendQueLen -= tmp.len;
			}
		}
	}

	if (m_sendQueLen < 0)
		m_sendQueLen = 0;

	if (execPos == len)
	{
		delete [] execBuf;
		return 0;
	}
	else
	{
		typedef int (WSAAPI *API_SEND)(SOCKET, const char*, int, int);
		int result = reinterpret_cast<API_SEND>(m_send)(s, execBuf, execLen, flags);

		delete [] execBuf;
		return result;
	}
}


