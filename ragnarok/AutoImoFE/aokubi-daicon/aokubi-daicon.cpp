#include "stdafx.h"


const BYTE RET264[4] = { 0x61, 0xC2, 0x08, 0x01 };

bool InjectModule(HANDLE hProcess, HANDLE hThread, LPTSTR lpFileName)
{
	LPBYTE lpBuffer = NULL;

	try
	{
		PathAppend(lpFileName, _T("AutoImoFE.dll"));

		CONTEXT context;
		context.ContextFlags = CONTEXT_CONTROL|CONTEXT_INTEGER;

		if (GetThreadContext(hThread, &context) == FALSE)
			throw 0x10;

		lpBuffer = new BYTE[48+MAX_PATH];

		context.Esp -= (48+MAX_PATH);
		*(LPDWORD)&lpBuffer[0] = context.Esp + 44;
		*(LPDWORD)&lpBuffer[4] = context.Esp + 48;
		*(LPDWORD)&lpBuffer[8] = context.Edi;
		*(LPDWORD)&lpBuffer[12] = context.Esi;
		*(LPDWORD)&lpBuffer[16] = context.Ebp;
		*(LPDWORD)&lpBuffer[20] = context.Esp + (48+MAX_PATH);
		*(LPDWORD)&lpBuffer[24] = context.Ebx;
		*(LPDWORD)&lpBuffer[28] = context.Edx;
		*(LPDWORD)&lpBuffer[32] = context.Ecx;
		*(LPDWORD)&lpBuffer[36] = context.Eax;
		*(LPDWORD)&lpBuffer[40] = context.Eip;
		CopyMemory(&lpBuffer[44], RET264, 4);
		//lstrcpy(reinterpret_cast<LPTSTR>(&lpBuffer[48]), lpFileName);
		CopyMemory(&lpBuffer[48], lpFileName, MAX_PATH);

		context.Eip = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandle(_T("kernel32")), _T("LoadLibraryA")));
		if (context.Eip == NULL)
			throw 0x20;

		DWORD dwWritten;
		if (WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(context.Esp), lpBuffer, 48+MAX_PATH, &dwWritten) == FALSE)
			throw 0x40;

		if (SetThreadContext(hThread, &context) == FALSE)
			throw 0x80;

		delete [] lpBuffer;
	}
	catch (int)
	{
		if (lpBuffer)
			delete [] lpBuffer;

		return false;
	}

	return true;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LPTSTR lpBasePath = new TCHAR[MAX_PATH];
	LPTSTR lpExeName = new TCHAR[MAX_PATH];

	GetModuleFileName(NULL, lpBasePath, MAX_PATH);
	PathRemoveFileSpec(lpBasePath);

	lstrcpy(lpExeName, lpBasePath);
	PathAppend(lpExeName, _T("AutoImo.exe"));

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if (CreateProcess(lpExeName, GetCommandLine(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, lpBasePath, &si, &pi) == FALSE)
		return 1;

	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof OSVERSIONINFO;
	GetVersionEx(&osv);

	if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT)
		InjectModule(pi.hProcess, pi.hThread, lpBasePath);
//	else
//		InjectModule9x(&pi, Buffer.get());

	ResumeThread(pi.hThread);
	CloseHandle(pi.hThread);
	WaitForInputIdle(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);

	delete [] lpExeName;
	delete [] lpBasePath;

	return 0;
}
