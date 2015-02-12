#include "chatlog.hpp"
#include <io.h>
#include <tchar.h>
#include <string.h>
#include <windows.h>	// ::Sleep()
#include "../ntdll_time.hpp"


ChatLog* ChatLog::m_this = 0;

////////////////////////////////////////////////////////////////////////////////


void ChatLog::initialize()
{
	m_this = this;

	NTDLL::NTDLLInitialize();

	unsigned int thread_id;
	m_thread = _beginthreadex(NULL, 0, &que_task_thread, NULL, 0, &thread_id);
}

void ChatLog::finalize()
{
	m_logQue.push(NULL);

	while (m_logQue.size())
	{
		const char* p = m_logQue.front();
		if (p)
			delete [] p;

		m_logQue.pop();
	}

	if (m_thread)
		::CloseHandle(reinterpret_cast<HANDLE>(m_thread));
}

////////////////////////////////////////////////////////////////////////////////


unsigned int ChatLog::que_task()
{
	while (1)
	{
		if (m_logQue.size())
		{
			const char* buffer = m_logQue.front();

			if (buffer == NULL)
			{
				m_logQue.pop();
				break;
			}
			else
			{
				size_t count = strlen(buffer);
				_write(m_fd, buffer, count);
				
				delete [] buffer;

				m_logQue.pop();
			}
		}
		else
			::Sleep(1*1000);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////


void ChatLog::output(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	int length = _vsctprintf(format, ap) + 1;
	TCHAR* buffer = new TCHAR[length];
	_vsntprintf(buffer, length, format, ap);
	m_logQue.push(buffer);
	
	va_end(ap);
}


void ChatLog::getTimeFields(void* p)
{
	TIME_FIELDS* tf = reinterpret_cast<TIME_FIELDS*>(p);
	LARGE_INTEGER st, lt;

	NtQuerySystemTime(&st);
	RtlSystemTimeToLocalTime(&st, &lt);
	RtlTimeToTimeFields(&lt, tf);
}


void ChatLog::normal(const BYTE* __buf)
{
	TIME_FIELDS tf;
	getTimeFields(&tf);
	output("NOR %02d:%02d:%02d %s\n", tf.Hour, tf.Minute, tf.Second, __buf);
}

void ChatLog::party(const BYTE* __buf)
{
	TIME_FIELDS tf;
	getTimeFields(&tf);
	output("PRT %02d:%02d:%02d %s\n", tf.Hour, tf.Minute, tf.Second, __buf);
}

void ChatLog::guild(const BYTE* __buf)
{
	TIME_FIELDS tf;
	getTimeFields(&tf);
	output("GLD %02d:%02d:%02d %s\n", tf.Hour, tf.Minute, tf.Second, __buf);
}

void ChatLog::wis_s(const BYTE* __name, const BYTE* __buf)
{
	TIME_FIELDS tf;
	getTimeFields(&tf);
	output("WIS %02d:%02d:%02d TO %s : %s\n", tf.Hour, tf.Minute, tf.Second, __name, __buf);
}

void ChatLog::wis_r(const BYTE* __name, const BYTE* __buf)
{
	TIME_FIELDS tf;
	getTimeFields(&tf);
	output("WIS %02d:%02d:%02d FROM %s : %s\n", tf.Hour, tf.Minute, tf.Second, __name, __buf);
}

void ChatLog::broadcast(const BYTE* __buf)
{
	TIME_FIELDS tf;
	getTimeFields(&tf);
	output("GOD %02d:%02d:%02d %s\n", tf.Hour, tf.Minute, tf.Second, __buf);
}

void ChatLog::lbc(const BYTE* __buf)
{
	TIME_FIELDS tf;
	getTimeFields(&tf);
	output("LBC %02d:%02d:%02d %s\n", tf.Hour, tf.Minute, tf.Second, __buf);
}

void ChatLog::talkie(const BYTE* __buf)
{
	TIME_FIELDS tf;
	getTimeFields(&tf);
	output("TLK %02d:%02d:%02d %s\n", tf.Hour, tf.Minute, tf.Second, __buf);
}

void ChatLog::sys_zone(const BYTE* __buf)
{
	TIME_FIELDS tf;
	getTimeFields(&tf);
	output("MAP %s (%04d-%02d-%02d %02d:%02d:%02d)\n", __buf, tf.Year, tf.Month, tf.Day, tf.Hour, tf.Minute, tf.Second);
}

void ChatLog::sys_char(const BYTE* __buf)
{
	output("SYS %s has connected\n", __buf);
}
