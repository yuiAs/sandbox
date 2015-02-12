#ifndef NETWORKBASE_HPP
#define NETWORKBASE_HPP


#include <winsock2.h>
#include <windows.h>
#include <tchar.h>


#pragma comment(lib, "ws2_32")


class CNetworkBase
{
	static CNetworkBase* m_this;

protected:

	void *m_pfnSend, *m_pfnRecv, *m_pfnConnect, *m_pfnInetAddr;

public:
	
	CNetworkBase();
	~CNetworkBase();

private:

	void initialize();

	// 抽象関数
	// 特にごそごそしないなら返値は0

	virtual int recv(unsigned char* buf, int length) = 0;
	virtual int send(unsigned char* buf, int length) = 0;

	// 仮想関数

	virtual int WSAAPI ws2_connect(SOCKET s, const struct sockaddr FAR * name, int namelen);
	virtual int WSAAPI ws2_recv(SOCKET s, char *buf, int len, int flags);
	virtual int WSAAPI ws2_send(SOCKET s, const char *buf, int len, int flags);
	virtual unsigned long WSAAPI ws2_inet_addr(const char* cp);

public:

	// あとは再定義不要

	static int WSAAPI _connect(SOCKET s, const struct sockaddr FAR * name, int namelen);
	static int WSAAPI _recv(SOCKET s, char *buf, int len, int flags);
	static int WSAAPI _send(SOCKET s, const char *buf, int len, int flags);
	static unsigned long WSAAPI _inet_addr(const char* cp);
};


#endif	// #ifndef NETWORKBASE_HPP
