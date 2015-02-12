#ifndef VC2005_H
#define VC2005_H
#if _MSC_VER < 1400

#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <io.h>
#include <errno.h>

typedef int errno_t;

////////////////////////////////////////////////////////////////////////////////


static errno_t _sopen_s(int* pfh, const char* filename, int oflag, int shflag, int pmode)
{
	if (pfh == NULL)
		return EINVAL;

	int fd = _open(filename, oflag, pmode);
	if (fd == -1)
	{
		*pfh = -1;
		return EINVAL;
	}

	*pfh = fd;
	return 0;
}


static int _snprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = _vsntprintf(buffer, count, format, ap);
	va_end(ap);

	return result;
}


static errno_t localtime_s(struct tm* _tm, const time_t* time)
{
	struct tm* t = localtime(time);
	if (t == NULL)
		return EINVAL;
	
	memcpy(_tm, &t, sizeof(struct tm));
	return 0;
}


#endif	// #if _MSC_VER < 1400
#endif	// #ifndef VC2005
