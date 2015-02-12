#include "ro_log.h"
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../client.h"

extern Client* cl;

////////////////////////////////////////////////////////////////////////////////


ROLog::ROLog() :
	m_worker(NULL), m_fd(-1), m_nxTime(-1)
{
}


ROLog::~ROLog()
{
	destroy();
}

////////////////////////////////////////////////////////////////////////////////


unsigned int __stdcall ROLog::worker_thread(void* parameter)
{
	return reinterpret_cast<ROLog*>(parameter)->worker();
}


unsigned int ROLog::worker()
{
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);

	while (1)
	{
		flush();
		::Sleep(0x0100);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


void ROLog::init(const char *dir, const char *prefix)
{
	aquire();

	unsigned int dummy;
	m_worker = _beginthreadex(NULL, 0, &worker_thread, reinterpret_cast<void*>(this), 0, &dummy);
}


void ROLog::destroy()
{
	::CloseHandle(reinterpret_cast<HANDLE>(m_worker));
	flush();

	if (m_fd > 0)
		_close(m_fd);
}

////////////////////////////////////////////////////////////////////////////////


void ROLog::push(unsigned char* buf, int buflen, enum RO_LOG_TYPE type)
{
	if (buflen == -1)
		buflen = strlen(reinterpret_cast<const char*>(buf)) + 1;

	LQUE tmp;

	tmp.type = type;
	tmp.timeVal = time(NULL);
	tmp.data.buf = new unsigned char[buflen];
	memcpy(tmp.data.buf, buf, buflen);

	m_logQue.push(tmp);
}


void ROLog::push(unsigned int val, enum RO_LOG_TYPE type)
{
	LQUE tmp;

	tmp.type = type;
	tmp.timeVal = time(NULL);
	tmp.data.val = val;

	m_logQue.push(tmp);
}

////////////////////////////////////////////////////////////////////////////////


void ROLog::formatted_que(LQUE* que)
{
	char timeBuf[TIMEBUF_SIZE];
	memset(timeBuf, 0, TIMEBUF_SIZE);

	{
		struct tm tmp;
#if _MSC_VER >= 1400
		localtime_s(&tmp, &que->timeVal);
#else
		memcpy(&tmp, localtime(&que->timeVal), sizeof(struct tm));
#endif
		_snprintf(timeBuf, TIMEBUF_SIZE, "%02d:%02d:%02d", tmp.tm_hour, tmp.tm_min, tmp.tm_sec);
	}

	char* buf = new char[MAX_BUFFERSIZE];
	int buflen = 0;

	switch (que->type)
	{
		case SYS_PLAIN:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "SYS %s %s\n", timeBuf, que->data.buf);
			delete [] que->data.buf;
			break;

		case SYS_MVP:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "MVP %s TARGET_AID=%08X\n", timeBuf, que->data.val);
			break;

		case CHAT_NOR:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "NOR %s %s\n", timeBuf, que->data.buf);
			delete [] que->data.buf;
			break;

		case CHAT_PRT:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "PRT %s %s\n", timeBuf, que->data.buf);
			delete [] que->data.buf;
			break;

		case CHAT_GLD:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "GLD %s %s\n", timeBuf, que->data.buf);
			delete [] que->data.buf;
			break;
	
		case CHAT_WIS_S:
			{
				const unsigned char* name = que->data.buf;
				const unsigned char* body = que->data.buf + 24;
				buflen = _snprintf(buf, MAX_BUFFERSIZE, "WIS %s TO %s : %s\n", timeBuf, name, body);
				delete [] que->data.buf;
			}
			break;

		case CHAT_WIS_R:
			{
				const unsigned char* p = que->data.buf;
				const unsigned char* name = p + 4;
				const unsigned char* body = p + 28;

				// [op:02][len:02][name:24] 28 29 30 31 [body:??]
				if (*reinterpret_cast<const unsigned char*>(p+2) > 32)
				{
					// 0x0FFFFFFFを超えるAIDは存在しないと仮定
					if (*reinterpret_cast<const unsigned char*>(p+31) < 0x10)
						body = p + 32;
				}

				buflen = _snprintf(buf, MAX_BUFFERSIZE, "WIS %s FROM %s : %s\n", timeBuf, name, body);
				delete [] que->data.buf;
			}
			break;

		case CHAT_WIS_E:
			{
				// line0149:  接続していないか、存在しないキャラクターの名前です。#
				// line0151:  全てのキャラクターに対して受信拒否状態です。#

				char* msg = NULL;

				switch (que->data.val)
				{
					case 0x01:
						cl->GetMsgString(MSG_WIS_FAILED, &msg);
						break;
					case 0x02:
						cl->GetMsgString(MSG_WIS_REFUSE, &msg);
						break;
				}

				if (msg)
					buflen = _snprintf(buf, MAX_BUFFERSIZE, "WIS %s\n", msg);
			}
			break;

		case CHAT_GBC:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "GBC %s %s\n", timeBuf, que->data.buf);
			delete [] que->data.buf;
			break;

		case CHAT_LBC:
			buflen = _snprintf(buf, MAX_BUFFERSIZE, "LBC %s %s\n", timeBuf, que->data.buf);
			delete [] que->data.buf;
			break;

		default:
			buflen = -1;
			break;
	}

	if (buflen > 0)
		write(buf, buflen);

	if (buf)
		delete [] buf;
}

////////////////////////////////////////////////////////////////////////////////


void ROLog::que_proc(LQUE* que)
{
	if (que->timeVal > m_nxTime)
		aquire();

	formatted_que(que);
}

////////////////////////////////////////////////////////////////////////////////


void ROLog::flush()
{
	while (m_logQue.size())
	{
		LQUE tmp = m_logQue.front();
		m_logQue.pop();
		que_proc(&tmp);
	}
}


void ROLog::aquire()
{
	if (m_fd > 0)
	{
		_close(m_fd);
		m_fd = -1;
	}

	time_t timeVal = time(NULL);

	struct tm curTime;
#if _MSC_VER >= 1400
	localtime_s(&curTime, &timeVal);
#else
	memcpy(&curTime, localtime(&timeVal), sizeof(struct tm));
#endif

	char fname[MAX_PATH];
	_snprintf(fname, MAX_PATH, "./Chat/%04d-%02d-%02d.txt", curTime.tm_year+1900, curTime.tm_mon+1, curTime.tm_mday);
	
	m_fd = _open(fname, _O_APPEND|_O_CREAT|_O_TEXT|_O_WRONLY, S_IWRITE);
	if (m_fd == -1)
	{
		m_nxTime = -1;
	}
	else
	{
		curTime.tm_hour = 0;
		curTime.tm_min = 0;
		curTime.tm_sec = 0;
		curTime.tm_wday = -1;
		curTime.tm_yday = -1;
		curTime.tm_isdst = -1;

		m_nxTime = mktime(&curTime) + 60*60*24;
	}
}


void ROLog::write(const char* buf, int buflen)
{
	if (m_fd > 0)
		_write(m_fd, buf, buflen);
}
