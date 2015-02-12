#include "RagnarokCore.hpp"
#include "Ragnarok.hpp"


////////////////////////////////////////////////////////////////////////////////

// skill_id(ASCII)取得

bool CRagnarokCore::get_skill_id(enum skill_id id, TCHAR* buf)
{
	if (m_address[AD_SKILLID] != 0)
	{
		::CopyMemory(buf, reinterpret_cast<const void*>(*reinterpret_cast<DWORD*>(m_address[AD_SKILLID]+id*4)), LN_SKILLID);
		return true;
	}

	return false;
}

// npc_id(ASCII)取得

bool CRagnarokCore::get_npc_id(int npc_id, TCHAR* buf)
{
	if (m_address[AD_NPCID] != 0)
	{
		// 動的確保なので念のため随時チェック
		DWORD address = *reinterpret_cast<DWORD*>(m_address[AD_NPCID]+4);
		::CopyMemory(buf, reinterpret_cast<const void*>(*reinterpret_cast<DWORD*>(address+npc_id*4)), LN_NAME_NPC);

		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

// systemwindowへ文字列を追加

bool CRagnarokCore::cl_pushText(const char* string, COLORREF color)
{
	if (m_address[AD_GAMEBASE] == 0)
		return false;
	if (m_address[AD_CALL_PUSHTEXT] == 0)
		return false;

	void* base = reinterpret_cast<void*>(m_address[AD_GAMEBASE]);
	void* func = reinterpret_cast<void*>(m_address[AD_CALL_PUSHTEXT]);

	__asm
	{
		mov ecx, string
		push 0x00000000
		push color
		push ecx
		push 0x00000001
		mov ecx, [base]
		call [func]
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////


bool CRagnarokCore::cl_pushSkillID(int n, const char* string)
{
	if (m_address[AD_SKILLID] == 0)
		return false;

	//DWORD t = *reinterpret_cast<DWORD*>(m_address[AD_SKILLID]) + n*4;
	::CopyMemory(reinterpret_cast<void*>(*reinterpret_cast<DWORD*>(m_address[AD_SKILLID])+n*4), &string, sizeof(DWORD));

	return true;
}

////////////////////////////////////////////////////////////////////////////////

// shotrcutwindowの表示非表示切り替え

bool CRagnarokCore::cl_showSCWnd(int showflag)
{
	if (m_address[AD_GAMEBASE] == 0)
		return false;
	if (m_address[AD_CALL_SHSCWND] == 0)
		return false;

	void* base = reinterpret_cast<void*>(m_address[AD_GAMEBASE]);
	void* func = reinterpret_cast<void*>(m_address[AD_CALL_SHSCWND]);

	__asm
	{
		push [showflag]
		push 0x24
		mov ecx, [base]
		call [func]
	}

	return true;
}

bool CRagnarokCore::is_showSCWnd()
{
	if (m_address[AD_SCWNDFLAG] == 0)
		return true;

	return *reinterpret_cast<DWORD*>(m_address[AD_SCWNDFLAG]) == 0x01;
}

////////////////////////////////////////////////////////////////////////////////


bool CRagnarokCore::cl_changeSC(const char* string, int lv, int pos)
{
	DWORD address = 0x00623480;
	void* func = reinterpret_cast<void*>(address);
	void* base = reinterpret_cast<void*>(m_address[AD_ACTORBASE]);

	__asm
	{
		mov ecx, string
		push pos
		push lv
		push ecx
		mov ecx, [base]
		call [func]
	}

	return true;
}


