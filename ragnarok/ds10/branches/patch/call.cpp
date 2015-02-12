#include "client.hpp"
#include <stdio.h>
#include <stdarg.h>

////////////////////////////////////////////////////////////////////////////////


namespace client
{
	enum { LINEBUF_SIZE=80 };

////////////////////////////////////////////////////////////////////////////////

// ctPrintf

HRESULT cl_ctprintf(const char* buf, COLORREF color)
{
	DWORD base = address(AD_GAME);
	DWORD func = address(AD_CL_CTPRINTF);

	if ((base==0) || (func==0))
		return E_POINTER;

	void* _base = reinterpret_cast<void*>(base);
	void* _func = reinterpret_cast<void*>(func);

	__asm
	{
		mov ecx, buf
		push 0
		push color
		push ecx
		push 1
		mov ecx, [_base]
		call [_func]
	}

	return S_OK;
}


HRESULT cl_ctprintf(COLORREF color, const char* fmt, ...)
{
	char buf[LINEBUF_SIZE];

	va_list ap;
	va_start(ap, fmt);
#if _MSC_VER >= 1400
	int result = _vsnprintf_s(buf, LINEBUF_SIZE, LINEBUF_SIZE-1, fmt, ap);
#else
	int result = _vsnprintf(buf, LINEBUF_SIZE-1, fmt, ap);
#endif
	va_end(ap);

	if (result > 0)
		return cl_ctprintf(buf, color);

	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////

// loadGrf

HRESULT cl_loadgrf(const char* filename)
{
	DWORD base = address(AD_CTRES);
	DWORD func = address(AD_CL_LOADGRF);

	if ((base==0) || (func==0))
		return E_POINTER;

	void* _base = reinterpret_cast<void*>(base);
	void* _func = reinterpret_cast<void*>(func);

	__asm
	{
		mov ecx, filename
		push ecx
		mov ecx, [base]
		call [func]
	}

	return S_OK;
}


};	// namespace client
