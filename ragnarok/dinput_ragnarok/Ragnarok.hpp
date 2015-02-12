#ifndef RAGNAROK_HPP
#define RAGNAROK_HPP


#include "sharedData.hpp"
#include "debugInfo.hpp"
#include "RagnarokCore.hpp"

#pragma warning(disable: 4312)	// greater size


class CRagnarok : public CRagnarokCore, public CDebugInfo
{
	static const TCHAR* PACKET_SL_ZONE;
	static const TCHAR* PACKET_SL_CHAR;

private:

	CSharedData<ACTOR_DATA>* m_sd;

	HINSTANCE m_instance;
	HWND m_wnd;
	HANDLE m_event[ENUM_SL];

	unsigned long m_charAddr;

public:

	CRagnarok(HINSTANCE instance) :
	  m_sd(0), m_instance(instance), m_wnd(0), m_charAddr(0) { initialize(); }
	~CRagnarok() { finalize(); }

private:

	void initialize()
	{
		m_sd = new CSharedData<ACTOR_DATA>(GLOBALNAME);

		m_event[SL_ZONE] = ::CreateEvent(NULL, TRUE, FALSE, PACKET_SL_ZONE);
		m_event[SL_CHAR] = ::CreateEvent(NULL, TRUE, FALSE, PACKET_SL_CHAR);
	}

	void finalize()
	{
		::CloseHandle(m_event[SL_ZONE]);
		::CloseHandle(m_event[SL_CHAR]);

		delete m_sd;
	}

public:

	// out

	unsigned long getActorVal(enum ACTOR order) const { return m_sd->get()->value[order]; }
	void getActorName(TCHAR* buf) { ::CopyMemory(buf, m_sd->get()->name, LN_NAME_CHAR); }
	unsigned long getCharIP() const { return m_charAddr; }

	HINSTANCE getInstance() const { return m_instance; }
	HWND getWnd() const { return m_wnd; }

	// in

	void setActorVal(enum ACTOR order, unsigned long val) { m_sd->get()->value[order] = val; }
	void setActorName(const TCHAR* name) { ::CopyMemory(m_sd->get()->name, name, LN_NAME_CHAR); }
	void setCharIP(unsigned long addr) { m_charAddr = addr; }

	void setWnd(HWND wnd) { m_wnd = wnd; }

	// event

	HANDLE getEvent(enum SIGNAL order) const { return m_event[order]; }
	void setEvent(enum SIGNAL order) { ::SetEvent(m_event[order]); }
	void resetEvent(enum SIGNAL order) { ::ResetEvent(m_event[order]); }
};


#endif	// #ifndef RAGNAROK_HPP

