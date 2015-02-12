#ifndef PE_JUMP_TABLE_HPP_D40DACD6_D934_49f0_89B4_56B9CA07A801
#define PE_JUMP_TABLE_HPP_D40DACD6_D934_49f0_89B4_56B9CA07A801

#include <windows.h>
#include <map>


class PEJmpTbl
{
	std::map<DWORD, DWORD> m_table;

public:

	PEJmpTbl() { clear(); }
	~PEJmpTbl() {}

public:

	bool isBuild() const { return m_table.empty()==false; }

	void build(DWORD start, DWORD end);
	void clear() { m_table.clear(); }
	DWORD search(const char* module, const char* exname);
	void* rewrite(const char* module, const char* exname, void* function);

private:

	void _protect(const void* address, MEMORY_BASIC_INFORMATION* memInfo);
	void _recover(MEMORY_BASIC_INFORMATION* memInfo);
};



#endif	// #ifnde PE_JUMP_TABLE_HPP
