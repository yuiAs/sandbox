#ifndef PEOPERATE_HPP_AE41C622_5E8A_4f38_A503_283AD8717F2B
#define PEOPERATE_HPP_AE41C622_5E8A_4f38_A503_283AD8717F2B

#include <windows.h>
#include "pe_jump_table.hpp"


class PEOp
{
	IMAGE_NT_HEADERS* m_NT;
	PEJmpTbl* m_jmpTbl;

public:

	PEOp(HMODULE module) : m_NT(NULL), m_jmpTbl(NULL) { initialize(module); }
	~PEOp();

private:

	void initialize(HMODULE module);

public:

	bool wait_ASProtect(DWORD loop_wait=1, DWORD expand_wait=0);

	// memory

	void copy(void* dest, const void* source, size_t length);

private:

	void _protect(const void* address, MEMORY_BASIC_INFORMATION* memInfo);
	void _recover(MEMORY_BASIC_INFORMATION* memInfo);

	// matching

	DWORD _match_first_at_data(const char* text);
	DWORD _match_first_at_text(const BYTE* buf, size_t buflen);

public:

	DWORD code(const BYTE* buf, size_t buflen);
	DWORD code_ref(const BYTE* buf, size_t buflen, int shift);
	DWORD string(const char* text);

public:

	void jmp_build();
	void jmp_clear();
	DWORD jmp_search(const char* module, const char* exname);
	void* jmp_rewrite(const char* module, const char* exname, void* function);

};


#endif	// #ifndef PEOPERATE_HPP
