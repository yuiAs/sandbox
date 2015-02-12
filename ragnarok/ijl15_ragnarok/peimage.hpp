#ifndef PEIMAGE_HPP
#define PEIMAGE_HPP

#include "stdafx.h"
#include <map>


class PEImage
{
	HMODULE m_handle;					// なくてもいいけど一応
	IMAGE_NT_HEADERS* m_nt;

	std::map<DWORD, DWORD> m_jmptbl;

public:

	PEImage(void* module) : m_handle(reinterpret_cast<HMODULE>(module)) { initialize(); }
	~PEImage() {}

private:

	void initialize();

	DWORD matchFirstByText(const BYTE* __data, size_t __length);
	DWORD matchFirstByData(const char* __string);

public:

	DWORD searchCode(const BYTE* data, size_t length);
	DWORD searchCodeRef(const BYTE* data, size_t length, size_t shift);
	DWORD searchString(const char* string);

	// VirtualProtect併用のRtlCopyMemory
	void copyMemory(void* dest, const void* source, size_t length);

	// ASProtectのimage復元完了チェック
	bool waitASProtect();

	void analysisFF25();														// jumptable解析
	void* rewriteFF25(const TCHAR* module, const char* export, void* function);	// jumptable修正
	bool isBuildFF25() { return m_jmptbl.empty()==false; }
	void clearFF25() { m_jmptbl.clear(); }
	void displayFF25();															// debug用
	DWORD searchFF25(const TCHAR* module, const char* export);

	void getSectionRange(int section, DWORD &start, DWORD &end);
};


#endif	// #ifndef PEIMAGE_HPP
