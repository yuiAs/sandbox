#include "client.h"
#include "dbgprint.h"

////////////////////////////////////////////////////////////////////////////////


bool Client::GetInstruction(DWORD address, DWORD range, DWORD* count, x86_insn_t* insn)
{
	DWORD st = *count;

	for (DWORD i=st; i<range;)
	{
		int result = x86_disasm(reinterpret_cast<u_char*>(address), range, 0, i, insn);
		if (result == 0)
		{
			i++;
			continue;
		}
		else
		{
			i += result;

			if (i > range)
				return false;
			else
			{
				*count = i;
				return true;
			}
		}
	}

	return false;
}


inline DWORD Client::GetCallAddress(DWORD address, x86_insn_t* insn)
{
	return address + insn->addr + insn->size + insn->operands->op.data.dword;
}

////////////////////////////////////////////////////////////////////////////////

// push n
// nÇÕ.data sectionì‡ÇÃaddress

DWORD Client::FindPushString(const char* text)
{
	BYTE code[] = {
		0x68, 0x00, 0x00, 0x00, 0x00,
	};
	*reinterpret_cast<DWORD*>(code+1) = findstr(text);

	return findbin(code, sizeof(code));
}

////////////////////////////////////////////////////////////////////////////////

// MOV [n], EAX
// [n]==valÇÃèÍçáÇ…nÇÃílÇm_address[type]Ç…ï€ë∂

void Client::search_pattern1(DWORD address, DWORD range, DWORD val, enum ADDRESS type)
{
	for (DWORD i=0; i<range;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range, &i, &insn) == false)
			break;

		if (insn.type != insn_mov)
			continue;
		if (insn.operands->op.data.reg.id != REG_EAX)
			continue;

		x86_operand_list* nx = insn.operands->next;
		if (nx->op.type == op_offset)
		{
			DWORD _val = *reinterpret_cast<DWORD*>(nx->op.data.dword);
			if (_val == val)
			{
				m_address[type] = nx->op.data.dword;
				break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


void Client::search_recv(DWORD range)
{
	DWORD address = FindPushString("recv");
	if (address != 0)
	{
		DWORD _api = reinterpret_cast<DWORD>(::GetProcAddress(::GetModuleHandle(_T("ws2_32.dll")), "recv"));
		search_pattern1(address, range, _api, AD_RECV);
	}
}


void Client::search_send(DWORD range)
{
	DWORD address = FindPushString("send");
	if (address != 0)
	{
		DWORD _api = reinterpret_cast<DWORD>(::GetProcAddress(::GetModuleHandle(_T("ws2_32.dll")), "send"));
		search_pattern1(address, range, _api, AD_SEND);
	}
}

////////////////////////////////////////////////////////////////////////////////


void Client::search_netbase(DWORD range)
{
	DWORD address = FindPushString("Sc_Notify_Ban - login");
	if (address == 0)
		return;

	dbgprintf(0, "%08X PUSH Sc_Notify_Ban - login\n", address);
	DWORD calladdr = 0;

	for (DWORD i=0; i<range;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range, &i, &insn) == false)
			break;

		if (insn.type != insn_call)
			continue;

		if (DWORD _tmp = GetCallAddress(address, &insn))
		{
			if (*reinterpret_cast<BYTE*>(_tmp) == 0xC3)
				continue;	// RETÇÃÇ›ÇÕdbgprintfÇÃÇÊÇ§Ç»Ç‡ÇÃ

			calladdr = _tmp;
			break;
		}
	}

	if (calladdr == 0)
		return;

	dbgprintf(0, "get_netbase=%08X\n", calladdr);

	void* func = reinterpret_cast<void*>(calladdr);
	DWORD _tmp = 0;

	__asm
	{
		call [calladdr]
		mov _tmp, eax
	};

	m_address[AD_NETBASE] = _tmp;
}

////////////////////////////////////////////////////////////////////////////////


void Client::search_drawbase(DWORD range)
{
	BYTE code[] = {
		0x6A, 0x00,		// PUSH 0
		0x6A, 0x00,		// PUSH 0
		0x6A, 0x00,		// PUSH 0
		0x6A, 0x03,		// PUSH 3
	};

	DWORD address = findbin(code, sizeof(code));
	if (address == 0)
		return;

	for (DWORD i=0; i<range;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range, &i, &insn) == false)
			break;

		if (insn.type == insn_mov)
		{
			if (insn.operands->op.data.reg.id != REG_ECX)
				continue;

			x86_operand_list* nx = insn.operands->next;
			if (nx->op.type == op_immediate)
				m_address[AD_DRAWBASE] = nx->op.data.dword;
		}

		if (insn.type == insn_call)
		{
			m_address[AD_CALL_CLPRT] = GetCallAddress(address, &insn);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


void Client::search_aid(DWORD range)
{
	DWORD address = FindPushString("PACKET_ZC_RESURRECTION");
	if (address == 0)
		return;

	dbgprintf(0, "%08X PUSH PACKET_ZC_RESURRECTION\n", address);

	for (DWORD i=0; i<range;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range, &i, &insn) == false)
			break;

		if (insn.type != insn_mov)
			continue;

		x86_operand_list* nx = insn.operands->next;
		if (nx->op.type == op_offset)
		{
			m_address[AD_AID] = nx->op.data.dword;
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


void Client::search_charname(DWORD range1, DWORD range2)
{
	DWORD address = FindPushString("%s : %s%s");
	if (address == 0)
		return;

	dbgprintf(0, "%%s : %%s%%s=%08X\n", address);
	address -= range1;

	DWORD calladdr = 0;

	for (DWORD i=0; i<range1;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range1, &i, &insn) == false)
			break;

		if (insn.type == insn_mov)
		{
			x86_operand_list* nx = insn.operands->next;
			if (nx->op.type == op_immediate)
				m_address[AD_ACTORBASE] = nx->op.data.dword;
		}

		if (insn.type == insn_call)
		{
			if (m_address[AD_ACTORBASE])
			{
                calladdr = GetCallAddress(address, &insn);
				break;
			}
		}
	}

	if (calladdr == 0)
		return;

	dbgprintf(0, "copy_charname=%08X\n", calladdr);

	for (DWORD i=0; i<range2;)
	{
		x86_insn_t insn;

		if (GetInstruction(calladdr, range2, &i, &insn) == false)
			break;

		if (insn.type != insn_mov)
			continue;
		if (insn.operands->op.data.reg.id != REG_EDI)
			continue;

		x86_operand_list* nx = insn.operands->next;
		if (nx->op.type == op_immediate)
		{
			m_address[AD_CHARNAME] = nx->op.data.dword;
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


void Client::search_msgTable(DWORD range)
{
	BYTE code[] = {
		0x68, 0x64, 0xFF, 0xFF, 0x00,	// PUSH 00FFFF64
	};

	DWORD address = findbin(code, sizeof(code));
	if (address == 0)
		return;

	for (DWORD i=0; i<range;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range, &i, &insn) == false)
			break;

		if (insn.type == insn_call)
		{
			m_address[AD_CALL_MSGTBL] = GetCallAddress(address, &insn);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


void Client::search_gamebase(DWORD range)
{
	DWORD address = FindPushString("login.rsw");
	if (address == 0)
		return;

	dbgprintf(0, "login.rsw=%08X\n", address);

	for (DWORD i=0; i<range;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range, &i, &insn) == false)
			break;

		if (insn.type != insn_mov)
			continue;
		if (insn.operands->op.data.reg.id != REG_ECX)
			continue;

		x86_operand_list* nx = insn.operands->next;
		if (nx->op.type == op_immediate)
		{
			m_address[AD_GAMEBASE] = nx->op.data.dword;
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


void Client::search_zoneParser(DWORD range)
{
	BYTE code[] = {
		0x83, 0xC0, 0x8D,	// ADD EAX, FFFFFF8D	; ADD EAX, -73
	};

	DWORD address = findbin(code, sizeof(code));
	if (address == 0)
		return;

	dbgprintf(0, "ADD EAX, FFFFFF8D=%08X\n", address);

	DWORD jmptable = 0;

	for (DWORD i=0; i<range;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range, &i, &insn) == false)
			break;

		if (insn.type == insn_jmp)
		{
			jmptable = insn.operands->op.data.expression.disp;
			break;
		}
	}

	if (jmptable == 0)
		return;

	dbgprintf(0, "zone_parser_jmptable=%08X\n", jmptable);
	m_address[AD_ZONE_JMPTBL] = jmptable;
	address = *reinterpret_cast<DWORD*>(jmptable+(0x0196-0x73)*4);
	dbgprintf(0, "zone_parser_jmp_0x0196=%08X\n", address);

	for (DWORD i=0; i<range;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range, &i, &insn) == false)
			break;

		if (insn.type == insn_jmp)
			break;
		if (insn.type == insn_call)
		{
			m_address[AD_CALL_ZP0196] = GetCallAddress(address, &insn);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////


void Client::search_APIJmpTable(DWORD range)
{
	BYTE code[] = {
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
		0x0C, 0xFF, 0XFF, 0x80, 0x00, 0x00, 0x00, 0x00,
	};

	DWORD address = findbin(code, sizeof(code));
	if (address == 0)
		return;

	dbgprintf(0, "api_jmptable=%08X\n", address);
	address += sizeof(code);

	for (DWORD i=0; i<range;)
	{
		x86_insn_t insn;

		if (GetInstruction(address, range, &i, &insn) == false)
			break;

		if (insn.type == insn_int)
			break;	// int 0x03Ç≈jumptableèIóπÇÃÇÕÇ∏
		if (insn.type != insn_jmp)
			break;

		// JMP DWORD PTR [n]
		// FF25n

		if (insn.operands->op.type == op_expression)
		{
			DWORD a = insn.operands->op.data.expression.disp;
			DWORD v = *reinterpret_cast<DWORD*>(a);
			m_importTable.insert(std::pair<DWORD, DWORD>(v, a));
		}
	}
}
