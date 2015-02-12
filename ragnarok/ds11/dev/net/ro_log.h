#ifndef RO_LOG_H_INCLUDED
#define RO_LOG_H_INCLUDED

#include <process.h>
#include <time.h>
#include <queue>
#include "ro_packet_enum.h"


class ROLog
{
	enum
	{
		MAX_BUFFERSIZE=1024,
		TIMEBUF_SIZE=16,
	};

	typedef struct tagLOGQUE
	{
		enum RO_LOG_TYPE type;
		time_t timeVal;
		union
		{
			unsigned int val;
			unsigned char* buf;
		} data;
	} LQUE, *PLQUE;

private:

	uintptr_t m_worker;

	int m_fd;
	time_t m_nxTime;

	std::queue<LQUE> m_logQue;

public:

	ROLog();
	~ROLog();

public:

	void init(const char* dir, const char* prefix);

	void push(unsigned char* buf, int buflen, enum RO_LOG_TYPE type);
	void push(unsigned int val, enum RO_LOG_TYPE type);

private:

	void destroy();
	void flush();
	void aquire();
	void write(const char* buf, int buflen);

	void formatted_que(LQUE* que);
	void que_proc(LQUE* que);

private:

	static unsigned int __stdcall worker_thread(void* prarameter);
	unsigned int worker();
};


#endif	// #ifndef RO_LOG_H_INCLUDED
