#include "packet.hpp"
#include "../ntdll_time.hpp"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////


void CPacket::chatlog_initialize()
{
	m_chat = new ChatLog;

	NTDLL::NTDLLInitialize();

	chatlog_aquire(getLocalSeconds());
}


void CPacket::chatlog_finalize()
{
	if (m_chat)
		delete m_chat;
	if (m_fd)
		_close(m_fd);
}


void CPacket::chatlog_aquire(ULONG ltm)
{
	if (m_fd)
		_close(m_fd);


	LARGE_INTEGER lt;
	RtlSecondsSince1970ToTime(ltm, &lt);

	TIME_FIELDS tf;
	RtlTimeToTimeFields(&lt, &tf);


	char* filename = new char[MAX_PATH];
	_snprintf(filename, MAX_PATH, "Chat\\%04d-%02d-%02d.txt", tf.Year, tf.Month, tf.Day);

	m_fd = _open(filename, _O_APPEND|_O_CREAT|_O_TEXT|_O_WRONLY, _S_IWRITE);
	m_chat->set_fd(m_fd);
	
	delete [] filename;


	m_breakSec = getSpecificSeconds(tf.Year, tf.Month, tf.Day) + 60*60*24;
}


void CPacket::chatlog_check()
{
	if (m_breakSec < getLocalSeconds())
		chatlog_aquire(m_breakSec);
}

////////////////////////////////////////////////////////////////////////////////


ULONG CPacket::getLocalSeconds()
{
	LARGE_INTEGER st;
	NtQuerySystemTime(&st);

	LARGE_INTEGER lt;
	RtlSystemTimeToLocalTime(&st, &lt);

	ULONG ltm;
	RtlTimeToSecondsSince1970(&lt, &ltm);

	return ltm;
}


ULONG CPacket::getSpecificSeconds(int year, int month, int day, int hour, int minute, int second)
{
	TIME_FIELDS tf;
	memset(&tf, 0, sizeof(TIME_FIELDS));

	tf.Year = year;
	tf.Month = month;
	tf.Day = day;
	tf.Hour = hour;
	tf.Minute = minute;
	tf.Second = second;

	return getSpecificSeconds(&tf);
}


ULONG CPacket::getSpecificSeconds(void* ptf)
{
	LARGE_INTEGER dt;
	RtlTimeFieldsToTime(reinterpret_cast<PTIME_FIELDS>(ptf), &dt);

	ULONG dtm;
	RtlTimeToSecondsSince1970(&dt, &dtm);

	return dtm;
}


