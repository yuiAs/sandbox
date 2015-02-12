#ifndef PACKETTABLE_HPP
#define PACKETTABLE_HPP


#include <windows.h>


class CPacketTable
{
	// define Node struct

	typedef struct
	{
		DWORD l;	// PTBL_NODE_PTR l;
		DWORD p;	// PTBL_NODE_PTR p;
		DWORD r;	// PTBL_NODE_PTR r;
		DWORD op;
		int length;
	} PTBL_NODE, *PTBL_NODE_PTR;

private:

	int* m_table;
	DWORD m_lastOp;

public:

	CPacketTable();
	~CPacketTable() { delete [] m_table; }


	void build(DWORD address, int pos) { _build(address, pos); }

	int length(WORD op) const 
	{
		if (op > m_lastOp)
			return 1;
		if (m_table[op] == 0)
			return 2;

		return m_table[op];
	}

	// table自体をコピーする時用

	//void get(int* buf) { ::CopyMemory(buf, m_table, sizeof(int)*(m_lastOp+1)); }
	int* get() const { return m_table; }
	unsigned long size() const { return m_lastOp+1; }

private:

	void _build(DWORD address, int pos);

	void searchTree(PTBL_NODE_PTR current);		// 2分木探索
	PTBL_NODE_PTR searchRoot(DWORD address);	// 手抜き自動検索
};


#endif	// #ifndef PACKETTABLE_HPP
