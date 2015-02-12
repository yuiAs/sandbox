#ifndef CHATLOG_H_58B4E3A4_1BD4_4218_AF8D_69292E88CFDC
#define CHATLOG_H_58B4E3A4_1BD4_4218_AF8D_69292E88CFDC

#include <queue>
#include <time.h>


class ChatLog
{
public:

	enum chat_type
	{
		CHAT_NORMAL,
		CHAT_NORMAL_OWN,
		CHAT_PARTY,
		CHAT_GUILD,
		CHAT_BC,
		CHAT_LBC,
		CHAT_WIS_R,
		CHAT_WIS_S,
		SYS_COMMON,
		SYS_ZONEIN,
		SYS_MAPCHANGE,
		SYS_MVP,
	};

private:

	enum { MAX_BUFFERSIZE=1024, };


	typedef struct tagCHATQUE
	{
		enum chat_type type;
		time_t val;
		unsigned long num;
		unsigned char* buf;
	} CHATQUE, *PCHATQUE;

	std::queue<CHATQUE> m_que;

	int m_fd;
	time_t m_next;

	uintptr_t m_task;

public:

	ChatLog();
	~ChatLog();

private:

	void init();
	void fin();

public:

	void push(unsigned char* buf, int buflen, enum chat_type type);
	void push(unsigned long num, enum chat_type type);

private:

	bool pop(CHATQUE& cq);

private:

	void aquire();
	void flush();

	void log_format(CHATQUE& cq);
	void log_switch(time_t target);
	void log_write(char* buf, size_t buflen);

public:

	static unsigned int __stdcall _task(void* parameter) { return reinterpret_cast<ChatLog*>(parameter)->task(); }

private:

	unsigned int task();

};



#endif	// #ifndec CHATLOG_H

