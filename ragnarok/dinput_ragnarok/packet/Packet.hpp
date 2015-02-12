#ifndef PACKET_HPP
#define PACKET_HPP


#include "NetworkBase.hpp"
#include "PacketParse.hpp"
#include "../Ragnarok.hpp"


class CPacket : public CNetworkBase
{
	static CPacket* m_this;	

	CRagnarok* m_core;
	CPacketParser* m_parser;

	void *m_pfnGetProcAddress;

	in_addr m_current_addr;

public:

	CPacket();
	~CPacket();

private:

	// CPacketŒÅ—L

	void initialize();
	void apihook();

private:

	// CNetworkBaseŒp³

	virtual int recv(BYTE* buf, int length) { return m_parser->parseRecv(buf, length); }
	virtual int send(BYTE* buf, int length) { return m_parser->parseSend(buf, length); }

	// Hook

	virtual int WSAAPI ws2_connect(SOCKET s, const struct sockaddr FAR * name, int namelen);
	virtual unsigned long WSAAPI ws2_inet_addr(const char* cp);

	// CPacketŒÅ—L

private:

	FARPROC k32_gpa(HMODULE hModule, LPCSTR lpProcName);

public:

	static FARPROC WINAPI _GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
		{ return CPacket::m_this->k32_gpa(hModule, lpProcName); }

};


#endif	// #ifndef PACKET_HPP
