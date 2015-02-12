#include "client.h"
#include "dbgprint.h"

////////////////////////////////////////////////////////////////////////////////


namespace tmp
{
	static Client* m_cl = NULL;
	static DWORD m_zpTable = 0;


////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////


DWORD InjectPacketProc(WORD op, void* func)
{
	__try
	{
		DWORD _tmp = m_zpTable + (op-0x73)*4;
		DWORD address = *reinterpret_cast<DWORD*>(_tmp);
		
		m_cl->memcpy_f(reinterpret_cast<void*>(_tmp), func, sizeof(DWORD);

		dbgprintf(0, "%08X %08X %08X\n", _tmp, address, *reinterpret_cast<DWORD*>(_tmp));

		return address;
	}
	__except (::GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
	{
		return 0;
	}
}


void InitPacket(void* client)
{
	m_cl = reinterpret_cast<Client*>(client);
	m_zpTable = m_cl->GetAddress(AD_ZONE_JMPTBL);
}


};	// namespace tmp