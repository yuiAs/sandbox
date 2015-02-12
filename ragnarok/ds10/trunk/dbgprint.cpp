#include "dbgprint.h"
#include <stdio.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>
#include <windows.h>

#ifndef DPRINTF_BUFSIZE
#define DPRINTF_BUFSIZE 1024
#endif

////////////////////////////////////////////////////////////////////////////////


namespace DbgPrint
{
	int m_dout = DOUT_API;
	int m_level = -1;

	int m_fd = -1;

////////////////////////////////////////////////////////////////////////////....


void init(int ot)
{
	m_dout = ot;

	if (m_dout == DOUT_FILE)
	{
		if (m_fd > 0)
			_close(m_fd);

#if _MSC_VER >= 1400
		errno_t e = _tsopen_s(&m_fd, _T("./debuglog.txt"), _O_APPEND|_O_CREAT|_O_TEXT|_O_WRONLY, _SH_DENYWR, _S_IWRITE);
		if (e != 0)
		{
			m_dout = DOUT_API;
			m_fd = -1;
		}
#else
		m_fd = _topen(_T("./debuglog.txt"), _O_APPEND|_O_CREAT|_O_TEXT|_O_WRONLY, _S_IWRITE);
		if (m_fd == -1)
			m_dout = DOUT_API;
#endif
	}
}

void fin()
{
	if (m_fd > 0)
		_close(m_fd);
}

////////////////////////////////////////////////////////////////////////////////


void dout_file(const TCHAR* buf, int buflen)
{
	if (m_fd > 0)
		_write(m_fd, buf, buflen);
}


void dprintf(int level, const TCHAR* fmt, ...)
{
	TCHAR buf[DPRINTF_BUFSIZE];

	va_list ap;
	va_start(ap, fmt);
#if _MSC_VER >= 1400
	int result = _vsntprintf_s(buf, DPRINTF_BUFSIZE, DPRINTF_BUFSIZE-1, fmt, ap);
#else
	int result = _vsntprintf(buf, DPRINTF_BUFSIZE-1, fmt, ap);
#endif
	va_end(ap);

	switch (m_dout)
	{
		case DOUT_API:
			::OutputDebugString(buf);
			break;

		case DOUT_FILE:
			if (result > 0)
				dout_file(buf, result);
			break;
	}
}


};	// namespace DbgPrint
