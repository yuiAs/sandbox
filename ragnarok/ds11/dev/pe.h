#ifndef PE_H_INCLUDED
#define PE_H_INCLUDED

#include <windows.h>
extern "C" {
#include "libdisasm/libdis.h"
}

////////////////////////////////////////////////////////////////////////////////


enum REGISTER_ID
{
	REG_MEM=0,
	REG_EAX,
	REG_ECX,
	REG_EDX,
	REG_EBX,
	REG_ESP,
	REG_EBP,
	REG_ESI,
	REG_EDI,
	REG_AL=17,
	REG_CL,
	REG_DL,
};

////////////////////////////////////////////////////////////////////////////////


class PEOp
{
	IMAGE_NT_HEADERS* m_nt;

public:

	PEOp();
	~PEOp();

public:

	void init(HMODULE module);
	void destroy();

public:

	void memcpy_f(void* dest, const void* src, size_t count);

private:

	void vprotect(const void* address, MEMORY_BASIC_INFORMATION* pmi);
	void vprotect_restore(MEMORY_BASIC_INFORMATION* pmi);

public:

	DWORD findstr(const char* text);
	DWORD findbin(const BYTE* buf, size_t buflen);

private:

	bool find_data(const char* text, DWORD* address);
	bool find_text(const BYTE* buf, size_t buflen, DWORD* address);
};


#endif	// #ifndef PE_H_INCLUDED
