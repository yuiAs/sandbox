#include "packettable.hpp"

////////////////////////////////////////////////////////////////////////////////


void CPacketTable::build(DWORD __address)
{
	if (m_table.size())
		return;

	NODE* root = getRoot(__address);

	if (root == NULL)			// root nodeÇ™Ç»Ç¢
		return;
	if (root->right == NULL)	// last nodeÇ™Ç»Ç¢
		return;

	m_lastop = root->right->op;

	search(root->parent);
}


void CPacketTable::search(NODE* __current)
{
	if (__current != NULL)
	{
		//if (__current->op <= m_lastop)
		m_table.insert(std::pair<WORD, int>(static_cast<WORD>(__current->op), __current->length));
		
		search(__current->left);
		search(__current->right);
	}
}


CPacketTable::NODE* CPacketTable::getRoot(DWORD __address)
{
	for (DWORD d=__address; d<__address+0x02FFFF; d+=0x04)
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
	std::map<WORD, int>::iterator i = m_table.find(__op);
	if (i != m_table.end())
		return i->second;

	return 2;	// íËã`Ç≥ÇÍÇƒÇ»Ç¢èÍçáÇÕsizeof(WORD)
}
