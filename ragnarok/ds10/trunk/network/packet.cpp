#include "../common.h"
#include "packet.hpp"
#include "define.h"

////////////////////////////////////////////////////////////////////////////////


CPacket::CPacket()
	: m_root(0), m_chat(NULL)
{
	m_chatFlag = 0;

	m_truesight = 0;
	m_currentCI = CI_NONE;
	m_blockSKUp = false;
	m_blockSTUp = false;
}


CPacket::~CPacket()
{
	SAFE_DELETE(m_chat);
}

////////////////////////////////////////////////////////////////////////////////


void CPacket::initialize(DWORD address, bool as)
{
	if (as)
		search(address);
	else
		m_root = *reinterpret_cast<DWORD*>(address);

#ifdef _DEBUG
	{
		NODE* last = reinterpret_cast<NODE*>(m_root)->right;
		u_long lastop = last->op;
		dbgprintf(0, "address=%08X root=%08X last=%08X lastop=%08X\n", address, m_root, last, lastop);

		for (u_long i=0; i!=lastop; i++)
			dbgprintf(0, "0x%04X=%d\n", i, getLength(reinterpret_cast<NODE*>(m_root)->parent, i));
	}
#endif

	loadConfig();
	//m_chat = new ChatLog(m_chatFlag);
	m_chat = new ChatLog;
}

////////////////////////////////////////////////////////////////////////////////


int CPacket::getLength(u_char* buf)
{
	if (m_root)
	{
		u_short op = _mkU16(buf);
		int length = getLength(reinterpret_cast<NODE*>(m_root)->parent, op);

#ifdef _DEBUG
		{
			if (length != FAILED_VALUE)
				dbgprintf(2, "op=%04X len=%d\n", op, length);
		}
#endif

		if (length != FAILED_VALUE)
			return length;
	}

	return 2;
}


int CPacket::getLength(NODE* node, u_short op)
{
	if (node == NULL)
		return FAILED_VALUE;

	if (static_cast<u_short>(node->op) == op)
		return node->length;
	else
	{
		int dummy = getLength(node->left, op);
		if (dummy == FAILED_VALUE)
			dummy = getLength(node->right, op);

		return dummy;
	}
}

////////////////////////////////////////////////////////////////////////////////


void CPacket::search(DWORD address)
{
	for (DWORD d=address; d<address+SEARCH_RANGE; d+=4)
	{
		if (*reinterpret_cast<DWORD*>(d) == 0x00004000)
		{
			NODE* n = reinterpret_cast<NODE*>(*reinterpret_cast<DWORD*>(d+4));

			if (::IsBadReadPtr(reinterpret_cast<const void*>(n->right), sizeof(NODE*)) == FALSE)
			{
				m_root = *reinterpret_cast<DWORD*>(d+4);
				dbgprintf(0, "plt=%08X plt_top=%08X root=%08X\n", d+4, d, m_root);
				break;
			}
		}
	}
}
