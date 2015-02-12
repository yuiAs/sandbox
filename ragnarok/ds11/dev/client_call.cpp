#include "client.h"
#include <stdarg.h>
#include "dbgprint.h"

////////////////////////////////////////////////////////////////////////////////


HRESULT Client::SetCondition(unsigned short _type, unsigned char _flag)
{
	return E_FAIL;
}


HRESULT Client::SetCondition(unsigned char* buf)
{
	DWORD _base = m_address[AD_GAMEBASE];
	DWORD _func = m_address[AD_CALL_ZP0196];

	if ((_base==0) || (_func==0))
		return E_POINTER;

	DWORD base = *reinterpret_cast<DWORD*>(_base+4);
	const void* func = reinterpret_cast<const void*>(_func);

	__asm
	{
		MOV ECX, buf
		PUSH ECX
		MOV ECX, base
		CALL [func]
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////


HRESULT Client::GetMsgString(int num, char** buf)
{
	if (m_address[AD_CALL_MSGTBL] == 0)
		return E_POINTER;

	char* tmp = NULL;

	DWORD _func = m_address[AD_CALL_MSGTBL];
	const void* func = reinterpret_cast<const void*>(_func);

	__asm
	{
		PUSH num
		CALL [func]
		ADD ESP, 4
		MOV tmp, EAX
	}

	*buf = tmp;

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////


HRESULT Client::clprintf(COLORREF color, const char* fmt, ...)
{
	char buf[LINEBUF_SIZE];
	// http://msdn2.microsoft.com/en-us/library/1kt27hek(VS.80).aspx
	// To ensure that there is room for the terminating null,
	// be sure that count is strictly less than the buffer length
	// and initialize the buffer to null prior to calling the function.
	memset(buf, 0, LINEBUF_SIZE);

	va_list ap;
	va_start(ap, fmt);
	int result = _vsnprintf(buf, LINEBUF_SIZE-1, fmt, ap);
	va_end(ap);

	if (result > 0)
		return clprintf(buf, color);

	return E_FAIL;
}


HRESULT Client::clprintf(const char* buf, COLORREF color)
{
	DWORD _base = m_address[AD_DRAWBASE];
	DWORD _func = m_address[AD_CALL_CLPRT];

	if ((_base==0) || (_func==0))
		return E_POINTER;

	const void* base = reinterpret_cast<const void*>(_base);
	const void* func = reinterpret_cast<const void*>(_func);

	__asm
	{
		MOV ECX, buf
		PUSH 0
		PUSH color
		PUSH ECX
		PUSH 1
		MOV ECX, [base]
		CALL [func]
	}

	return S_OK;
}
