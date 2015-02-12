#include "packettable.hpp"

////////////////////////////////////////////////////////////////////////////////


void CPacketTable::build(DWORD address)
{
	if (m_table)
		return;

	NODE* root = getRoot(address);

	if (root == NULL)			// root node‚ª‚È‚¢
		return;
	if (root->right == NULL)	// last node‚ª‚È‚¢
		return;

	m_lastop = root->right->op;

	m_table = new int[m_lastop+1];
	memset(m_table, 0, m_lastop+1);

	search(root->parent);
}


void CPacketTable::search(NODE* current)
{
	if (current != NULL)
	{
		WORD op = static_cast<WORD>(current->op);
		if (op <= m_lastop)
			m_table[op] = current->length;
		
		search(current->left);
		search(current->right);
	}
}


CPacketTable::NODE* CPacketTable::getRoot(DWORD address)
{
	for (DWORD d=address; d<address+0x02FFFF; d+=0x04)
	{
		if (*reinterpret_cast<DWORD*>(d) != 0x00004000)
			continue;

		NODE* root = reinterpret_cast<NODE*>(*reinterpret_cast<DWORD*>(d+4));
		if (::IsBadReadPtr(reinterpret_cast<void*>(root->right), sizeof(NODE*)) == TRUE)
			continue;

		return root;
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////


int CPacketTable::get(WORD __op)
{
	if (__op <= m_lastop)
		return m_table[__op];

	return sizeof(WORD);
}
