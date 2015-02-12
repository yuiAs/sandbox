#ifndef RAGNAROKCORE_HPP
#define RAGNAROKCORE_HPP


#include <windows.h>
#include <tchar.h>
#include "Ragnarok.h"
#include "_skill_id.h"


class CRagnarokCore
{
private:

	DWORD m_address[ENUM_ADDR];

public:

	CRagnarokCore() {}
	~CRagnarokCore() {}

public:

	// 入出力関係

	DWORD getAddr(enum ADDR order) const { return m_address[order]; }
	void setAddr(enum ADDR order, DWORD val) { m_address[order] = val; }

public:

	bool get_skill_id(enum skill_id id, TCHAR* buf);
	bool get_npc_id(int npc_id, TCHAR* buf);

	bool is_showSCWnd();

	// クライアント依存系

	bool cl_pushText(const char* string, COLORREF color);
	bool cl_showSCWnd(int showflag);
	bool cl_pushSkillID(int n, const char* string);

	bool cl_changeSC(const char* string, int lv, int pos);

};


#endif	// #ifndef RAGNAROKCORE_HPP
