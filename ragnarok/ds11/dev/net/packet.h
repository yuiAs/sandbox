#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include <winsock2.h>


class Packet
{
	enum { MAX_BUFFERSIZE=2048, };

private:

	u_char* m_backBuf;
	int m_backLen;
	int m_backSet;

public:

	Packet();
	virtual ~Packet();

public:

	virtual int recv(char* buf, int buflen);
	virtual int send(const char* buf, int buflen);

private:

	bool prc_restbuf(u_char* buf, int buflen, int* pos, int* rest);

private:

	void inc_backbuf(int require);
	void clr_backbuf();

private:

	virtual int parse_r(u_char* buf, int buflen) = 0;
	virtual int parse_s(u_char* buf, int buflen) = 0;

	virtual int get_length(u_char* buf) = 0;
};


#endif	// #ifndef PACKET_H_INCLUDED
