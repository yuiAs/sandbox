#pragma once

#include <atlstr.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi")


namespace APIPLUS
{
	// API Overrap
	static void OutputDebugStringEx(LPCTSTR lpszFormat, ...);

	// API extended
	static BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nValue, LPCTSTR lpFileName, int nBufferSize=16);
	static BOOL PathReplaceFileSpec(LPTSTR pszDstPath, LPCTSTR pszFile);
}

////////////////////////////////////////////////////////////////////////////////


void APIPLUS::OutputDebugStringEx(LPCTSTR lpszFormat, ...)
{
	typedef CStringT<TCHAR, StrTraitATL<TCHAR> > CAtlString;
	CAtlString debugString;
	va_list argList;

	va_start(argList, lpszFormat);
	debugString.FormatV(lpszFormat, argList);
	OutputDebugString(debugString);
    va_end(argList);
}

////////////////////////////////////////////////////////////////////////////////

// Shelwapi extention

// PathReplaceFileSpec(LPTSTR pszDstPath, LPCTSTR pszFile)

BOOL APIPLUS::PathReplaceFileSpec(LPTSTR pszDstPath, LPCTSTR pszFile)
{
	if (::PathRemoveFileSpec(pszDstPath))
		return ::PathAppend(pszDstPath, pszFile);
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

// PrivateProfile control

// WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nValue, LPCTSTR lpFileName)
// lpAppName	= section name
// lpKeyName	= key name
// nValue		= value to add
// lpFileName	= initialization file
// nBufferSize

BOOL APIPLUS::WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nValue, LPCTSTR lpFileName, int nBufferSize)
{
	if (nBufferSize > 1024)
		return FALSE;

	LPTSTR Buffer = new char[nBufferSize];

	wsprintf(Buffer, "%d", nValue);
	BOOL bResult = ::WritePrivateProfileString(lpAppName, lpKeyName, Buffer, lpFileName);

	delete [] Buffer;
	return bResult;
}
