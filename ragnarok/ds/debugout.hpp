#ifndef DEBUGOUT_HPP
#define DEBUGOUT_HPP


#include <windows.h>
#include <atlfile.h>
#define _ATL_CSTRING_NO_CRT
#include <atlstr.h>


class CDebugOut
{

public:

	enum { DEBUGOUT_NONE, DEBUGOUT_STRING, DEBUGOUT_FILE };

private:

	int _out_type;
	CAtlFile _file;

public:

	CDebugOut(int type=DEBUGOUT_STRING) : _out_type(type)
	{
		if (_out_type == DEBUGOUT_FILE)
			_output_initialize();
	}

	~CDebugOut() {}

public:

	virtual void debugout(TCHAR* format, ...)
	{
		CAtlString ds;
		va_list ap;

		va_start(ap, format);
		ds.FormatV(format, ap);
		va_end(ap);

		if (_out_type == DEBUGOUT_STRING)
			_output_string(ds);
		else if (_out_type == DEBUGOUT_FILE)
			_output_file(ds);
	}

private:

	virtual void _output_initialize()
	{
		if (SUCCEEDED(_file.Create(_T("debug.txt"), GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS)))
		{
			SYSTEMTIME st;
			::GetLocalTime(&st);
			debugout(_T("----- %04d-%02d-%02d %02d:%02d:%02d -----"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		}
		else
			_out_type = DEBUGOUT_STRING;
	}

	//virtual void _output_string(const TCHAR* string) { ::OutputDebugString(string); }
	//virtual void _output_file(const TCHAR* string) { _file.Write(string, lstrlen(string)); }
	virtual void _output_string(CAtlString& string) { ::OutputDebugString(string); }
	virtual void _output_file(CAtlString& string) { _file.Write(static_cast<const TCHAR*>(string), string.GetLength()); }
};


#endif // #ifndef DEBUGOUT_HPP
