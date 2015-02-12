#ifndef CHATLOG_HPP
#define CHATLOG_HPP


#include "../outputFile.hpp"
#include <atlstr.h>


class CChatLog : public COutputFile
{
	SYSTEMTIME m_st;

	// enum

	enum { LN_CTRLSTRING=32 };

public:

	enum CHAT 
	{
		CT_ZONEINFO, CT_CHANGEMAP, CT_CHANGEZONE,
		CT_CREATECHATROOM, CT_JOINCHATROOM, CT_CHADDMEMBER, CT_CHLEFTMEMBER,
		CT_MVP, CT_TALKIE,

		CT_NORMALCHAT, CT_NORMALCHAT_OWN,
		CT_SENDWHISPER, CT_RECVWHISPER, CT_FAILEDWHISPER, 
		CT_BROADCAST, CT_LOCALBROADCAST, CT_PTCHAT, CT_GUILDCHAT,
	};

private:
	// string container
	typedef CStringT<TCHAR, StrTraitATL<TCHAR> > CAtlString;

public:

	CChatLog() { _acquire(NULL); }
	~CChatLog() {}

private:

	void _acquire(SYSTEMTIME* lpst);
	bool _checkdate(SYSTEMTIME* lpst);

	__inline SYSTEMTIME _gettime(SYSTEMTIME* lpst)
	{
		if (lpst == NULL)
		{
			SYSTEMTIME t;
			::GetLocalTime(&t);
			return t;
		}
		else
			return *lpst;
	}

	void adjust(const TCHAR* format, ...);
	//void _output(TCHAR* p, DWORD length) { write(p, length); }

public:

	void output(enum CHAT type, BYTE* buf, ...);

};


#endif	// #ifndef CHATLOG_HPP