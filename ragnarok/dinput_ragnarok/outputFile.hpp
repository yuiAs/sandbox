#ifndef OUTPUTFILE_HPP
#define OUTPUTFILE_HPP


#include <windows.h>
#include <tchar.h>


class COutputFile
{
	HANDLE _handle;

public:

	COutputFile() : _handle(NULL) { }
	~COutputFile() { release(); }

public:

	bool acquire(TCHAR* filename, int creation=OPEN_ALWAYS)
	{
		_handle = ::CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
		if (_handle == INVALID_HANDLE_VALUE)
			_handle = NULL;

		return alive();
	}

	DWORD write(const void* p, DWORD length)
	{
		if (_handle != NULL)
		{
			DWORD written;
			::WriteFile(_handle, p, length, &written, NULL);

			return written;
		}

		return 0;
	}

	void release() { ::CloseHandle(_handle); }
	bool alive() const { return _handle!=NULL; }
	HANDLE get() const { return _handle; }

};


#endif	// #ifndef OUTPUTFILE_HPP
