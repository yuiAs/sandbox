#ifndef PACKETTABLE_HPP
#define PACKETTABLE_HPP

#include <map>
#include <windows.h>
/*
typedef unsigned long DWORD;
typedef unsigned int WORD;
*/

class CPacketTable
{
	typedef struct tag_node
	{
		struct tag_node * left;
		struct tag_node * parent;
		struct tag_node * right;
//		DWORD left;
//		DWORD parent;
//		DWORD right
		DWORD op;
		int length;
	} NODE, *PNODE;
	
private:

	DWORD m_lastop;
	std::map<WORD, int> m_table;

public:

	CPacketTable() : m_lastop(-1) { m_table.clear(); }
	virtual ~CPacketTable() {}

public:

	void build(DWORD __address);

	int get(WORD __op);
	
	bool isBuild() { return m_table.empty()==false; }
	WORD last() { return static_cast<WORD>(m_lastop); }

private:

	NODE* getRoot(DWORD __address);
	void search(NODE* __current);

};


#endif	// #ifndef PACKETTABLE_HPP
