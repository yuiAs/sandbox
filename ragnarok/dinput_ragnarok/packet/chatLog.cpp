#include "chatLog.hpp"


////////////////////////////////////////////////////////////////////////////////


void CChatLog::_acquire(SYSTEMTIME* lpst)
{
	SYSTEMTIME t = _gettime(lpst);

	TCHAR* filename = new TCHAR[MAX_PATH];
	_sntprintf(filename, MAX_PATH, _T("Chat\\%04d-%02d-%02d.txt"), t.wYear, t.wMonth, t.wDay);

	if (acquire(filename) == true)
	{
		::SetFilePointer(get(), 0, NULL, FILE_END);
		m_st = t;
	}

	delete [] filename;
}


bool CChatLog::_checkdate(SYSTEMTIME* lpst)
{
	SYSTEMTIME t = _gettime(lpst);

	if ((t.wYear != m_st.wYear) || (t.wMonth != m_st.wMonth) || (t.wDay != m_st.wDay))
	{
		release();
		_acquire(&t);
	}

	return alive();
}

////////////////////////////////////////////////////////////////////////////////


void CChatLog::adjust(const TCHAR* format, ...)
{
	CAtlString cs;
	va_list ap;

	va_start(ap, format);
	cs.FormatV(format, ap);
	va_end(ap);

	write(reinterpret_cast<const void*>(static_cast<const TCHAR*>(cs)), cs.GetLength());
}


void CChatLog::output(enum CHAT type, BYTE* buf, ...)
{
	SYSTEMTIME t = _gettime(NULL);

	if (_checkdate(&t) == false)
		return;	// ファイル関連の障害ぐらい...
 

	va_list ap;
	va_start(ap, buf);


	switch (type)
	{
		case CT_ZONEINFO:
			{
				int p1 = va_arg(ap, int);	// mapname
				TCHAR* name = va_arg(ap, TCHAR*);
				adjust(_T("SYS %s has connected\r\n"), name);
				adjust(_T("MAP %s (%04d-%02d-%02d %02d:%02d:%02d)\r\n"), buf+p1, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
			}
			break;

		case CT_CHANGEMAP:
		case CT_CHANGEZONE:
			{
				int p1 = va_arg(ap, int);	// mapname
				adjust(_T("MAP %s (%04d-%02d-%02d %02d:%02d:%02d)\r\n"), buf+p1, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
			}
			break;

		case CT_MVP:
			{
				DWORD AID = va_arg(ap, DWORD);	// AID
				adjust(_T("MVP %02d:%02d:%02d AID=%08X\r\n"), t.wHour, t.wMinute, t.wSecond, AID);
			}
			break;

		case CT_NORMALCHAT:
		case CT_NORMALCHAT_OWN:
			{
				int p1 = va_arg(ap, int);	// body
				adjust(_T("%s %02d:%02d:%02d %s\r\n"), _T("NOR"), t.wHour, t.wMinute, t.wSecond, buf+p1);
			}
			break;

		case CT_SENDWHISPER:
			{
				int p1 = va_arg(ap, int);	// name
				int p2 = va_arg(ap, int);	// body
				adjust(_T("WIS %02d:%02d:%02d TO %s : %s\r\n"), t.wHour, t.wMinute, t.wSecond, buf+p1, buf+p2);
			}
			break;

		case CT_RECVWHISPER:
			{
				int p1 = va_arg(ap, int);	// name
				int p2 = va_arg(ap, int);	// body
				adjust(_T("WIS %02d:%02d:%02d FROM %s : %s\r\n"), t.wHour, t.wMinute, t.wSecond, buf+p1, buf+p2);
			}
			break;

		case CT_BROADCAST:
			{
				int p1 = va_arg(ap, int);	// body
				adjust(_T("%s %02d:%02d:%02d %s\r\n"), _T("GOD"), t.wHour, t.wMinute, t.wSecond, buf+p1);
			}
			break;

		case CT_PTCHAT:
			{
				int p1 = va_arg(ap, int);	// body
				adjust(_T("%s %02d:%02d:%02d %s\r\n"), _T("PRT"), t.wHour, t.wMinute, t.wSecond, buf+p1);
			}
			break;

		case CT_GUILDCHAT:
			{
				int p1 = va_arg(ap, int);	// body
				adjust(_T("%s %02d:%02d:%02d %s\r\n"), _T("GLD"), t.wHour, t.wMinute, t.wSecond, buf+p1);
			}
			break;

		case CT_TALKIE:
			{
				int p1 = va_arg(ap, int);	// body
				adjust(_T("%s %02d:%02d:%02d %s\r\n"), _T("TLK"), t.wHour, t.wMinute, t.wSecond, buf+p1);
			}
			break;

		case CT_LOCALBROADCAST:
			{
				int p1 = va_arg(ap, int);	// body
				adjust(_T("%s %02d:%02d:%02d %s\r\n"), _T("LBC"), t.wHour, t.wMinute, t.wSecond, buf+p1);
			}
			break;
	}


	va_end(ap);
}

