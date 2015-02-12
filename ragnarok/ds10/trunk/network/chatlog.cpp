#include "chatlog.hpp"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <share.h>
#include "../common.h"
#include <process.h>

#pragma warning(disable: 4996)

////////////////////////////////////////////////////////////////////////////////


ChatLog::ChatLog()
	: m_fd(-1), m_next(-1), m_task(0)
{
	init();
}


ChatLog::~ChatLog()
{
	fin();
}

////////////////////////////////////////////////////////////////////////////////


void ChatLog::init()
{
	aquire();

	unsigned int thread_id;
	m_task = _beginthreadex(NULL, 0, &_task, reinterpret_cast<void*>(this), 0, &thread_id);

	dbgprintf(0, "tid=%08X handle=%08X\n", thread_id, m_task);
}

void ChatLog::fin()
{
	::CloseHandle(reinterpret_cast<HANDLE>(m_task));
	flush();

	if (m_fd > 0)
		_close(m_fd);
}

////////////////////////////////////////////////////////////////////////////////


void ChatLog::push(unsigned char* buf, int buflen, enum chat_type type)
{
	if (buflen == -1)
		buflen = ::lstrlen(reinterpret_cast<const char*>(buf)) + 1;

	CHATQUE cq;

	cq.type = type;
	cq.val = time(NULL);
	cq.num = static_cast<unsigned long>(-1);
	cq.buf = new unsigned char[buflen];
	memcpy(cq.buf, buf, buflen);

	m_que.push(cq);
}


void ChatLog::push(unsigned long num, enum chat_type type)
{
	CHATQUE cq;

	cq.type = type;
	cq.val = time(NULL);
	cq.num = num;
	cq.buf = NULL;

	m_que.push(cq);
}


bool ChatLog::pop(CHATQUE& cq)
{
	if (m_que.empty())
		return false;

	cq = m_que.front();
	m_que.pop();
	return true;
}

////////////////////////////////////////////////////////////////////////////////


void ChatLog::log_format(CHATQUE& cq)
{
	char timebuf[64];

	{
		struct tm t;
		localtime_s(&t, &cq.val);
		_snprintf_s(timebuf, 64, 64-1, "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
	}

	unsigned char* cp = cq.buf;
	int cn = cq.num;

	char* buf = new char[MAX_BUFFERSIZE];
	int buflen = 0;

	switch (cq.type)
	{
		case CHAT_NORMAL:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "NOR %s %s\n", timebuf, cp+8);
			break;

		case CHAT_NORMAL_OWN:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "NOR %s %s\n", timebuf, cp+4);
			break;

		case CHAT_PARTY:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "PRT %s %s\n", timebuf, cp+8);
			break;

		case CHAT_GUILD:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "GLD %s %s\n", timebuf, cp+4);
			break;

		case CHAT_BC:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "GOD %s %s\n", timebuf, cp+4);
			break;

		case CHAT_LBC:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "LBC %s %s\n", timebuf, cp+16);
			break;

		case CHAT_WIS_R:
			{
				char* name = reinterpret_cast<char*>(cp+4);
				char* body = reinterpret_cast<char*>(cp+28);

				// [op:02][len:02][name:24] 28 29 30 31 [body??]
				if (_mkU16(cp+2) > 32)
				{
					// 0x00FFFFFFà»è„ÇÃAIDÇÕë∂ç›ÇµÇ»Ç¢Ç∆âºíË
					if (_mkU8(cp+31) == 0x00)
						body = reinterpret_cast<char*>(cp+28+4);
				}
				buflen = _snprintf(buf, MAX_BUFFERSIZE, "WIS %s FROM %s : %s\n", timebuf, name, body);
			}
			break;

		case CHAT_WIS_S:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "WIS %s TO %s : %s\n", timebuf, cp+4, cp+28);
			break;

		case SYS_ZONEIN:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "SYS %s %s connected.\n", timebuf, cp);
			break;

		case SYS_MVP:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "MVP %s aid=%08X\n", timebuf, cn);
			break;

		default:
			break;
	}

	if (buflen > 0)
		log_write(buf, buflen);

	if (cp)
		delete [] cp;
	if (buf)
		delete [] buf;
}

////////////////////////////////////////////////////////////////////////////////


void ChatLog::log_switch(time_t val)
{
	if (val > m_next)
		aquire();
}

void ChatLog::log_write(char* buf, size_t buflen)
{
	dbgprintf(1, "len=%3d %s", buflen, buf);

	if (m_fd > 0)
		_write(m_fd, buf, buflen);
}

////////////////////////////////////////////////////////////////////////////////


void ChatLog::aquire()
{
	if (m_fd > 0)
		_close(m_fd);

	time_t tt = time(NULL);

	struct tm t;
	localtime_s(&t, &tt);

	char fname[MAX_PATH];
	_snprintf_s(fname, MAX_PATH, MAX_PATH-1, "./Chat/%04d-%02d-%02d.txt", t.tm_year+1900, t.tm_mon+1, t.tm_mday);

	errno_t e = _sopen_s(&m_fd, fname, _O_APPEND|_O_CREAT|_O_TEXT|_O_WRONLY, _SH_DENYWR, _S_IWRITE);
	if (e != 0)
	{
		m_next = -1;
		m_fd = -1;
	}
	else
	{
		t.tm_hour = 0;
		t.tm_min = 0;
		t.tm_sec = 0;
		t.tm_wday = -1;
		t.tm_yday = -1;
		t.tm_isdst = -1;

		m_next = mktime(&t) + 60*60*24;
	}

	dbgprintf(0, "%s\n", fname);
}


void ChatLog::flush()
{
	while (m_que.size())
	{
		CHATQUE cq = m_que.front();
		log_switch(cq.val);
		log_format(cq);
		m_que.pop();
	}
}

////////////////////////////////////////////////////////////////////////////////


unsigned int ChatLog::task()
{
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);

	while (1)
	{
		if (m_que.size())
			flush();
		::Sleep(1);
	}

	return 0;
}
