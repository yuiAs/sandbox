#ifndef FILEOUT_H
#define FILEOUT_H

#include <wchar.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////


class FileOut
{
	int m_fh;

public:

	FileOut() : m_fh(-1) {}
	FileOut(wchar_t* FileName) : m_fh(-1) { Open(FileName); }

	virtual ~FileOut() { Close(); }

public:

	bool Open(wchar_t* FileName)
	{
		if (m_fh > 0)
			Close();
		return (m_fh=_wopen(FileName, _O_APPEND|_O_BINARY|_O_CREAT|_O_RDWR, _S_IREAD|_S_IWRITE))!=-1;
	}

	void Close()
	{
		_close(m_fh);
		m_fh = -1;
	}

	int Write(const void* Data, size_t Length)
	{
		return (m_fh>0)? _write(m_fh, Data, Length) : 0;
	}
/*
	long Tell()
	{
		// Šú‘Ò‚·‚é‚Ì‚ª•Ô‚Á‚Ä‚±‚È‚¢
		return (m_fh>0)? _tell(m_fh) : 0;
	}
*/
	long Size()
	{
		long Result = 0;
		if (m_fh > 0)
		{
			Result = _filelength(m_fh);
/*
			long Offset = _tell(m_fh);
			Result = _lseek(m_fh, 0, SEEK_END);
			_lseek(m_fh, Offset, SEEK_SET);
*/
		}
		return Result;
	}

	bool IsOpend() { return (m_fh>0); }

private:

#ifndef SEEK_SET
	enum { SEEK_SET=0, SEEK_CUR, SEEK_END };
#endif
};


#endif	// #ifndef FILEOUT_H
