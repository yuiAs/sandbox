#include "Shortcut.hpp"


CShortcut* CShortcut::m_this = 0;

////////////////////////////////////////////////////////////////////////////////


CShortcut::CShortcut(DWORD thread_id) : m_keyhook(0), m_zoneEv(0), m_bm(false)
{
	m_this = this;

	extern CRagnarok* core;
	m_core = core;

	initialize(thread_id);
}

////////////////////////////////////////////////////////////////////////////////


void CShortcut::initialize(DWORD thread_id)
{
	m_keyhook = ::SetWindowsHookEx(WH_KEYBOARD, KBProc, m_core->getInstance(), thread_id);
	m_zoneEv = m_core->getEvent(SL_ZONE);
}


void CShortcut::finalize()
{
	if (m_keyhook)
		::UnhookWindowsHookEx(m_keyhook);
}

////////////////////////////////////////////////////////////////////////////////


LRESULT CShortcut::keybord_proc(int code, WPARAM wParam, LPARAM lParam)
{
	if ((code < 0) || (code == HC_NOREMOVE))
		return ::CallNextHookEx(m_keyhook, code, wParam, lParam);


	if (::WaitForSingleObject(m_zoneEv, 0) == WAIT_OBJECT_0)
	{
/*
		if (wParam == VK_SCROLL)
		{
			if ((lParam & (1<<31)) == (1<<31))
				battleMode_toggle();

			return TRUE;
		}
*/
		if (wParam == VK_DIVIDE)
		{
			if (((lParam & (1<<30)) == 0) && ((lParam & (1<<31)) == 0))
				battleMode_toggle();

			return TRUE;
		}

		if (wParam == VK_F1)
		{
			if ((lParam & (1<<30)) == 0)
				m_core->cl_changeSC(_T("##WZ_STORMGUST"), 10, 7);
		}

		if (m_bm)
			return battleMode_proc(code, wParam, lParam);
	}


	return ::CallNextHookEx(m_keyhook, code, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////


void CShortcut::setPage(DWORD page)
{
	if (DWORD address = m_core->getAddr(AD_SCPAGE))
	{
		switch (page)
		{
			case 0:
				page = 2;
				break;
			case 1:
				page = 0;
				break;
			case 2:
				page = 1;
				break;
			default:
				__assume(0);
		}

		::CopyMemory(reinterpret_cast<void*>(address), &page, sizeof(DWORD));
	}
}

DWORD CShortcut::getPage()
{
	if (DWORD address = m_core->getAddr(AD_SCPAGE))
	{
		DWORD value;
		::CopyMemory(&value, reinterpret_cast<const void*>(address), sizeof(DWORD));

		return value;
	}

	return -1;
}

void CShortcut::sendKey(int key, bool check)
{
	HWND wnd = m_core->getWnd();
	bool show = m_core->is_showSCWnd();

	::SendMessage(wnd, WM_KEYDOWN, key, 0);
	//::keybd_event(key, 0, 0, 0);
	if (check && (show == false))
		m_core->cl_showSCWnd(0);
	::SendMessage(wnd, WM_KEYUP, key, 0);
	//::keybd_event(key, KEYEVENTF_KEYUP, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////


void CShortcut::battleMode_toggle()
{
	if (m_bm)
	{
		m_bm = false;
		m_core->cl_pushText(_T("BMが解除されました。NumPadの[/]で有効になります。"), CL_TEXT_NOTICE5);
	}
	else
	{
		m_bm = true;
		m_core->cl_pushText(_T("BMが有効になりました。NumPadの[/]で解除できます。"), CL_TEXT_NOTICE5);
	}
}


LRESULT CShortcut::battleMode(int vkey, DWORD page, bool keymode)
{
	if (keymode)
	{
		DWORD now = getPage();		// 0/1/2が返る
	
		if (now == page)
			sendKey(vkey, false);
		else
		{
			setPage(page);
			sendKey(VK_F12, true);

			sendKey(vkey, false);

			setPage(now);
			sendKey(VK_F12, true);
		}
	}

	return TRUE;
}


LRESULT CShortcut::battleMode_proc(int code, WPARAM wParam, LPARAM lParam)
{
	if (lParam & (1<<29))	// Altが押されている場合
		return ::CallNextHookEx(m_keyhook, code, wParam, lParam);
/*
	if (lParam & (1<<30))	// keystroke is repeated grater than 1
		return TRUE;
	
	if ((wParam < 0x30) || (wParam > 0x5A))
	{
		if ((wParam < VK_OEM_1) || (wParam > VK_OEM_8))
			return ::CallNextHookEx(m_keyhook, code, wParam, lParam);
	}
*/
	if ((wParam < 0x30) || (wParam > 0x5A))
	{
		if ((wParam < VK_OEM_1) || (wParam > VK_OEM_8))
			return ::CallNextHookEx(m_keyhook, code, wParam, lParam);
	}
	else
	{
		if (lParam & (1<<30))	// keystroke is repeated grater than 1
			return TRUE;
	}

	bool keymode = ((lParam & (1<<31)) == 0);		// WM_KEYDOWN=true


	switch (wParam)
	{
/*
		case VK_RETURN:
		case VK_SNAPSHOT:
			break;
*/
		// qwertyuio
		case 0x51:
			return battleMode(VK_F1, 2, keymode);
		case 0x57:
			return battleMode(VK_F2, 2, keymode);
		case 0x45:
			return battleMode(VK_F3, 2, keymode);
		case 0x52:
			return battleMode(VK_F4, 2, keymode);
		case 0x54:
			return battleMode(VK_F5, 2, keymode);
		case 0x59:
			return battleMode(VK_F6, 2, keymode);
		case 0x55:
			return battleMode(VK_F7, 2, keymode);
		case 0x49:
			return battleMode(VK_F8, 2, keymode);
		case 0x4F:
			return battleMode(VK_F9, 2, keymode);

		// asdfghjkl
		case 0x41:
			return battleMode(VK_F1, 1, keymode);
		case 0x53:
			return battleMode(VK_F2, 1, keymode);
		case 0x44:
			return battleMode(VK_F3, 1, keymode);
		case 0x46:
			return battleMode(VK_F4, 1, keymode);
		case 0x47:
			return battleMode(VK_F5, 1, keymode);
		case 0x48:
			return battleMode(VK_F6, 1, keymode);
		case 0x4A:
			return battleMode(VK_F7, 1, keymode);
		case 0x4B:
			return battleMode(VK_F8, 1, keymode);
		case 0x4C:
			return battleMode(VK_F9, 1, keymode);

		// zxcvbnm,.
		case 0x5A:
			return battleMode(VK_F1, 0, keymode);
		case 0x58:
			return battleMode(VK_F2, 0, keymode);
		case 0x43:
			return battleMode(VK_F3, 0, keymode);
		case 0x56:
			return battleMode(VK_F4, 0, keymode);
		case 0x42:
			return battleMode(VK_F5, 0, keymode);
		case 0x4E:
			return battleMode(VK_F6, 0, keymode);
		case 0x4D:
			return battleMode(VK_F7, 0, keymode);
		case 0xBC:
			return battleMode(VK_F8, 0, keymode);
		case 0xBE:
			return battleMode(VK_F9, 0, keymode);

		default:
			return TRUE;
			//break;
	}


	return ::CallNextHookEx(m_keyhook, code, wParam, lParam);
}


