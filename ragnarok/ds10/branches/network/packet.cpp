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
		NODE* st = reinterpret_cast<NODE*>(m_root)->left;
		NODE* ed = reinterpret_cast<NODE*>(m_root)->right;
		dbgprintf(0, "root=%08X st=%08X ed=%08X stop=%04X edop=%04X\n", m_root, st, ed, st->op, ed->op);

		for (u_long i=st->op; i!=ed->op; i++)
			dbgprintf(0, "0x%04X=%d\n", i, getLength(reinterpret_cast<NODE*>(m_root)->parent, i));

//		NODE* node = reinterpret_cast<NODE*>(m_root);
//		dbgprintf(0, "%08X %08X %08X %08X %d %04X %d\n", node, node->parent, node->left, node->right, node->color, node->op, node->length);
//		getLength(reinterpret_cast<NODE*>(m_root)->parent, 0x64);
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

	const u_short curOp = static_cast<const u_short>(node->op);

//	dbgprintf(0, "%08X %08X %08X %08X %d %04X %d\n", node, node->parent, node->left, node->right, node->color, node->op, node->length);

	if (curOp == op)
		return node->length;
	else
	{
		if (curOp > op)
			return getLength(node->left, op);
		if (curOp < op)
			return getLength(node->right, op);
	}

	return FAILED_VALUE;
}

////////////////////////////////////////////////////////////////////////////////


void CPacket::search(DWORD address)
{
	for (DWORD d=address; d<address+SEARCH_RANGE; d+=4)
	{
		__try
		{
			DWORD cur = *reinterpret_cast<DWORD*>(d);
			NODE* tmp = reinterpret_cast<NODE*>(cur);

			if (tmp == NULL)
				continue;
			if (tmp->color != 0)
				continue;
			if (tmp->left == NULL)
				continue;

			if (tmp->left->color == 1)
			{
				m_root = cur;
				dbgprintf(0, "plt=%08X plt_top=%08X root=%08X\n", d, d-4, m_root);
				break;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			continue;
		}
	}
}
