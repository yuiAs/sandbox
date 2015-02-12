#ifndef CHATLOG_HPP
#define CHATLOG_HPP

#include <queue>
#include <process.h>


class ChatLog
{

	static ChatLog* m_this;

private:

	int m_fd;			// 当面は単一ファイルへの書き出しのみ
	std::queue<const char*> m_logQue;

	uintptr_t m_thread;

public:

	ChatLog() : m_fd(-1), m_thread(0) { initialize(); }
	virtual ~ChatLog() { finalize(); }

	void set_fd(int fd) { m_fd = fd; }

private:

	void initialize();
	void finalize();

	// logging

	void output(const char* format, ...);
	void getTimeFields(void* p);

public:

	typedef unsigned char BYTE;
	typedef unsigned short WORD;
	typedef unsigned long DWORD;

	void normal(const BYTE* __buf);
	void party(const BYTE* __buf);
	void guild(const BYTE* __buf);
	void wis_s(const BYTE* __name, const BYTE* __buf);
	void wis_r(const BYTE* __name, const BYTE* __buf);
	void broadcast(const BYTE* __buf);
	void lbc(const BYTE* __buf);
	void talkie(const BYTE* __buf);

	void sys_mvp(DWORD integer) {}
	void sys_zone(const BYTE* __buf);
	void sys_char(const BYTE* __buf);

private:

	// thread

	unsigned int que_task();
	static unsigned int __stdcall que_task_thread(void* parameter)
	{ return ChatLog::m_this->que_task(); }


};


#endif	// #ifndef CHATLOG_HPP
