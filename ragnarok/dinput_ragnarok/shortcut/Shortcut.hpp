#ifndef SHORTCUT_HPP
#define SHORTCUT_HPP


#include <windows.h>
#include "../Ragnarok.hpp"


class CShortcut
{
	static CShortcut* m_this;

	CRagnarok* m_core;
	HHOOK m_keyhook;
	HANDLE m_zoneEv;

	bool m_bm;

public:

	CShortcut(DWORD thread_id);
	~CShortcut() { finalize(); }

public:

	void initialize(DWORD thread_id);
	void finalize();

private:

	void setPage(DWORD page);
	DWORD getPage();

	void sendKey(int key, bool check);

	LRESULT battleMode_proc(int code, WPARAM wParam, LPARAM lParam);
	void battleMode_toggle();
	LRESULT battleMode(int vkey, DWORD page, bool keymode);

private:

	LRESULT keybord_proc(int code, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK KBProc(int code, WPARAM wParam, LPARAM lParam)
		{ return CShortcut::m_this->keybord_proc(code, wParam, lParam); }
};


#endif	// #ifndef SHORTCUT_HPP
