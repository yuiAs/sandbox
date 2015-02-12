#include "client.hpp"
#include "../pe_operate.hpp"

extern PEOp* pe;

////////////////////////////////////////////////////////////////////////////////


namespace client
{
	DWORD _address[AD_MAX];

////////////////////////////////////////////////////////////////////////////////


template<typename T> inline T ref(DWORD address)
{
	if (::IsBadReadPtr(reinterpret_cast<const void*>(address), sizeof(T)))
		return 0;
	return *reinterpret_cast<T*>(address);
}

inline DWORD call(DWORD address)
{
	// eip = address + 4
	return address + 4 + ref<DWORD>(address);
}

////////////////////////////////////////////////////////////////////////////////

// AID

void _aid()
{
	BYTE code[] =
	{
		0x89, 0x5D, 0xFC,	// mov [ebp-04], ebx
		0x89, 0x55, 0xF8,	// mov [ebp-08], edx
	};

	_address[AD_AID] = pe->code_ref(code, sizeof(code), -4);
	dbgprintf(1, "AD_AID=%08X\n", _address[AD_AID]);
}

////////////////////////////////////////////////////////////////////////////////

// game base

// push 00000000		; 6A00
// push 0096FFFF		; 68FFFF9600
// push ecx				; 51
// push 00000001		; 6A01
// mov ecx, ________	; B9________
// call ________		; E8________

// mov ecx, ________	; B9________
// push 00000000		; 6A00
// push 0096FFFF		; 68FFFF9600
// push edx				; 52
// push 00000001		; 6A01
// call ________		; E8________

void _gamebase()
{
	BYTE code[] =
	{
		0x68, 0xFF, 0xFF, 0x96, 0x00,	// push 0096FFFF
	};

	DWORD address = pe->code(code, sizeof(code));
	DWORD _base=0, _shift=0;

	for (int i=-7; i<32;)
	{
		switch (*reinterpret_cast<BYTE*>(address+i))
		{
			case 0x50:	// push eax
			case 0x51:	// push ecx
			case 0x52:	// push edx
				i += 1;
				break;
			case 0x6A:	// push +
				i += 2;
				break;
			case 0xB9:	// mov ecx,
				_base = address + i + 1;
				i += 5;
				break;
			case 0xE8:	// call
				_shift = address + i + 1;
				i += 0xFF;					// callÇÃÇ∆Ç´ÇÕÇ«Ç§å©ÇƒÇ‡åüçıèIóπÇ≈Ç∑
				break;

			default:
				i++;
				break;
		}

		if (_base & _shift)
			break;
	}

	if (_base & _shift)
	{
		_address[AD_GAME] = ref<DWORD>(_base);
		_address[AD_CL_CTPRINTF] = call(_shift);
	}
	else
	{
		_address[AD_GAME] = 0;
		_address[AD_CL_CTPRINTF] = 0;
	}

/*
	DWORD address = pe->code(code, sizeof(code));
	DWORD _base=0, _shift=0;

	switch (*reinterpret_cast<BYTE*>(address+8))
	{
		case 0xB9:
			_base = address + 9;
			_shift = address + 14;
			break;

		case 0xE8:
			_base = address - 6;
			_shift = address + 9;
			break;

		default:
			_address[AD_GAME] = 0;
			_address[AD_CL_CTPRINTF] = 0;
			return;
	}

	_address[AD_GAME] = ref<DWORD>(_base);
	_address[AD_CL_CTPRINTF] = _shift + 4 + ref<DWORD>(_shift);	// eip=_shift+4,
*/
	dbgprintf(1, "AD_GAME=%08X\n", _address[AD_GAME]);
	dbgprintf(1, "AD_CL_CTPRINTF=%08X\n", _address[AD_CL_CTPRINTF]);
}

////////////////////////////////////////////////////////////////////////////////

// actor base

// push ebp				; 55
// mov ebp, esp			; 8BEC
// mov eax. [ebp+08]	; 8B4508
// mov ecx, ________	; B9________

void _actorbase()
{
	BYTE code[] = {
		0x55,				// push ebp
		0x8B, 0xEC,			// mov ebp, esp
		0x8B, 0x45, 0x08,	// mov eax, [ebp+08]
		0xB9,				// mov ecx,
	};

	_address[AD_ACTOR] = pe->code_ref(code, sizeof(code), 7);
	dbgprintf(1, "AD_ACTOR=%08X\n", _address[AD_ACTOR]);
}

////////////////////////////////////////////////////////////////////////////////

// control resource

// push ********		; 68********
// mov ecx, ________	; B9________
// call ________		; E8________

void _ctrlresource()
{
	BYTE push[] = {
		0x68, 0x00, 0x00, 0x00, 0x00,	// push 00000000
	};

	DWORD string = pe->string("data.grf");
	*reinterpret_cast<DWORD*>(push+1) = string;
	DWORD address = pe->code(push, sizeof(push));

//	DWORD _res = address + 6;
//	_address[AD_CTRES] = ref<DWORD>(address+6);
	DWORD _res=0, _shift=0;

	for (int i=-5; i<32;)
	{
		switch (*reinterpret_cast<BYTE*>(address+i))
		{
			case 0xB9:	// mov ecx,
				_res = address + i + 1;
				i += 5;
				break;
			case 0xE8:	// call
				_shift = address + i + 1;
				i += 0xFF;
				break;

			default:
				i++;
				break;
		}

		if (_res & _shift)
			break;
	}

	if (_res & _shift)
	{
		_address[AD_CTRES] = ref<DWORD>(_res);
		_address[AD_CL_LOADGRF] = call(_shift);
	}
	else
	{
		_address[AD_CTRES] = 0;
		_address[AD_CL_LOADGRF] = 0;
	}

/*
	for (int i=10; i<32; i++)
	{
		if (*reinterpret_cast<BYTE*>(address+i) == 0xE8)
		{
			_shift = address + i + 1;
			break;
		}
	}
*/
/*
	if (*reinterpret_cast<BYTE*>(address+10) == 0xE8)
		_shift = address + 11;
	else
	{
		for (int i=10; i<30; i++)
		{
			if (*reinterpret_cast<BYTE*>(address+i) == 0xE8)
			{
				_shift = address + i + 1;
				break;
			}
		}
	}
*/
/*
	if (_shift == 0)
		_address[AD_CL_LOADGRF] = 0;
	else
		_address[AD_CL_LOADGRF] = _shift + 4 + ref<DWORD>(_shift);
*/
	dbgprintf(2, "AD_CTRES=%08X\n", _address[AD_CTRES]);
	dbgprintf(2, "AD_CL_LOADGRF=%08X\n", _address[AD_CL_LOADGRF]);
}

////////////////////////////////////////////////////////////////////////////////

// network base

// push eax			; 50
// push 000001ED	; 68ED010000
// call ________	; E8________

void _netbase()
{
	BYTE code[] = {
		0x50,							// push eax
		0x68, 0xED, 0x01, 0x00, 0x00,	// push 000001ED
	};

	DWORD address = pe->code(code, sizeof(code));
//	DWORD _shift = ref<DWORD>(address+7);
//	void* _func = reinterpret_cast<void*>(address+11+_shift);
	address += 6+1;
	void* _func = reinterpret_cast<void*>(call(address));
	DWORD _netbase = 0;

	__asm
	{
		call [_func]
		mov _netbase, eax
	}

	_address[AD_NET] = _netbase;
	dbgprintf(1, "AD_NET=%08X\n", _address[AD_NET]);
}

////////////////////////////////////////////////////////////////////////////////

// recv/send

void _ws2call()
{
	BYTE jmp[] = {
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,	// jmp 00000000
	};

	// send

	{
		DWORD address = pe->jmp_search("ws2_32.dll", "send");
		*reinterpret_cast<DWORD*>(jmp+2) = address;
		DWORD target = pe->code(jmp, sizeof(jmp));

		_address[AD_SEND] = pe->code_ref(reinterpret_cast<const BYTE*>(&target), sizeof(DWORD), -4);
	}

	// recv

	{
		DWORD address = pe->jmp_search("ws2_32.dll", "recv");
		*reinterpret_cast<DWORD*>(jmp+2) = address;
		DWORD target = pe->code(jmp, sizeof(jmp));

		_address[AD_RECV] = pe->code_ref(reinterpret_cast<const BYTE*>(&target), sizeof(DWORD), -4);
	}

	dbgprintf(1, "AD_SEND=%08X\n", _address[AD_SEND]);
	dbgprintf(1, "AD_RECV=%08X\n", _address[AD_RECV]);
}

////////////////////////////////////////////////////////////////////////////////

// character name

// inc eax				; 40
// cmp eax, 00000040	; 83F840
// jb ********			; 72**
// push edi				; 5F
// push esi				; 5E
// push eax ________	; B8________

void _charname()
{
	BYTE code[] = {
		0x40,				// inc eax
		0x83, 0xF8, 0x40,	// cmp eax, 00000040
		0x72,				// jb
	};

	DWORD address = pe->code(code, sizeof(code));

	if (*reinterpret_cast<BYTE*>(address+8) == 0xB8)	// push eax nnnnnnnn
	{
		_address[AD_NAME] = ref<DWORD>(address+9);
		dbgprintf(1, "AD_NAME=%08X\n", _address[AD_NAME]);
	}
}

////////////////////////////////////////////////////////////////////////////////

// IDirectDraw7

// mov ecx, ________			; B9________
// mov dword ptr [ebp-64], 0	; C7459C00000000
// call ________				; E8________

// 05 B9xxxxxxxx
// 07 C7459C00000000
// 05 E8xxxxxxxx
// 02 85C0
// 02 7Dxx

void _iface_dd7()
{
	BYTE push[] = {
		0x68, 0x00, 0x00, 0x00, 0x00,	// push 00000000
	};

	DWORD string = pe->string("Cannot init d3d OR grf file has problem.");
	*reinterpret_cast<DWORD*>(push+1) = string;
	DWORD address = pe->code(push, sizeof(push));
}

////////////////////////////////////////////////////////////////////////////////

// FontBase

// mov ecx, ________	; B9________
// call ********		; E8********
// push 00406040		; 6840604000

void _fontbase()
{
	BYTE push[] = {
		0x68, 0x00, 0x00, 0x00, 0x00,	// push 00000000
	};

	*reinterpret_cast<DWORD*>(push+1) = 0x00406040;
	DWORD address = pe->code(push, sizeof(push));

	if (address == 0)
	{
		*reinterpret_cast<DWORD*>(push+1) = 0x004060D0;
		address = pe->code(push, sizeof(push));
	}

	_address[AD_FONT] = ref<DWORD>(address-9);
	dbgprintf(1, "AD_FONT=%08X\n", _address[AD_FONT]);
}

////////////////////////////////////////////////////////////////////////////////


void search()
{
	for (int i=0; i<AD_MAX; i++)
		_address[i] = 0;

	_aid();
	_gamebase();
	_actorbase();
//	_ctrlresource();
	_charname();
	_netbase();
	_ws2call();
//	_iface_dd7();
	_fontbase();
}


DWORD address(enum ADDRESS type)
{
	return _address[type];
}


};	// namespace client
