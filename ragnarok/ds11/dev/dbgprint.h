#ifndef DBGPRINT_H_A1B8C628_1AB1_4b14_8FEB_47055BDEDCE7
#define DBGPRINT_H_A1B8C628_1AB1_4b14_8FEB_47055BDEDCE7

#include <tchar.h>


namespace DbgPrint
{
	enum out_type
	{
		DOUT_NONE,
		DOUT_API,
		DOUT_FILE,
	};

	////////////////////////////////////////////////////////////////////////////

	void init(int ot=DOUT_API);
	void fin();

	void dprintf(int level, const TCHAR* fmt, ...);
};	// namespace DbgPrint


#ifndef dbgprintf
#ifdef _DEBUG
#define dbgprintf DbgPrint::dprintf
#else
#define dbgprintf __noop
#endif
#endif


#endif	// #ifndef DBGPRINT_H


