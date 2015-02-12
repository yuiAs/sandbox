#include "client.hpp"
#include "../pe_operate.hpp"

extern PEOp* pe;

////////////////////////////////////////////////////////////////////////////////


namespace client
{

////////////////////////////////////////////////////////////////////////////////

// unprotect sakray cleint

void unprotect_sakray()
{
	// old 00050505050102030505050504
	// new 00050105050502030505050504
	//     0 1 2 3 4 5 6 7 8 9 A B C

	BYTE code[] = {
		0x02, 0x03, 0x05, 0x05, 0x05, 0x05, 0x04,
	};

	DWORD address = pe->code(code, sizeof(code)) - 6;
	if (address == 0)
		return;

 	if (::IsBadReadPtr(reinterpret_cast<void*>(address), 13) == 0)
	{
		if (*reinterpret_cast<BYTE*>(address+2) != 0x05)
		{
			BYTE m = 0x05;
			pe->copy(reinterpret_cast<void*>(address+2), &m, sizeof(BYTE));
		}
	}
}

// fix HFONT

void fix_hfont(enum ADDRESS type, void* font)
{
	DWORD base = address(type);
	if (base == 0)
		return;

    DWORD stack = *reinterpret_cast<DWORD*>(base+0x04);
	void* address = reinterpret_cast<void*>(stack+0x10);

	if (::IsBadWritePtr(address, sizeof(HFONT)) == 0)
		pe->copy(address, font, sizeof(HFONT));
}


};	// namespace client
