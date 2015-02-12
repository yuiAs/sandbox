#ifndef DEBUG_INFO_HPP
#define DEBUG_INFO_HPP


#include <windows.h>
#include <tchar.h>
#include <atlstr.h>


class CDebugInfo
{
public:
	// èoóÕêÊ
	enum { OUT_NONE, OUT_STRING, OUT_FILE };

private:
	// string container
	typedef CStringT<TCHAR, StrTraitATL<TCHAR> > CAtlString;

private:

	int _output_type;

public:

	CDebugInfo(int output_type=OUT_STRING) : _output_type(output_type) {}
	~CDebugInfo() {}

public:

	void output(TCHAR* format, ...)
	{
		CAtlString dstring;
		va_list ap;

		va_start(ap, format);
		dstring.FormatV(format, ap);
		va_end(ap);

		if (_output_type == OUT_STRING)
			output_string(dstring);
		else if (_output_type == OUT_FILE)
			output_file(dstring);
	}

private:

	// OutputDebugString()

	void output_string(const TCHAR* string) { ::OutputDebugString(string); }

	// debuglog.txtÇ…èoóÕ

	void output_file(const TCHAR* string)
	{
	}

};


#endif	// #ifndef DEBUG_INFO_HPP
