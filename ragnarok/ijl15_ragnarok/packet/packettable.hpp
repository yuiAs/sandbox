#ifndef PACKETTABLE_HPP
#define PACKETTABLE_HPP

#include <windows.h>


class CPacketTable
{
	typedef struct tag_node
	{
		struct tag_node * left;
		struct tag_node * parent;
		struct tag_node * right;
		DWORD op;
		int length;
	} NODE, *PNODE;
	
private:

	DWORD m_lastop;
	int* m_table;

public:

	CPacketTable() : m_lastop(-1), m_table(NULL) {}
	virtual ~CPacketTable()
	{
		if (m_table)
			delete [] m_table;
	}

public:

	void build(DWORD address);

	int get(WORD op);
	
	bool isBuild() { return m_table!=NULL; }
	WORD last() { return static_cast<WORD>(m_lastop); }

private:

	NODE* getRoot(DWORD address);
	void search(NODE* current);

};


#endif	// #ifndef PACKETTABLE_HPP
