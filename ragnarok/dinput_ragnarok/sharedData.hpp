#ifndef SHAREDDATA_HPP
#define SHAREDDATA_HPP


#include <windows.h>
#include <tchar.h>


template <class T> 
class CSharedData
{
	T* _data;
	HANDLE _handle;

public:

	CSharedData(const TCHAR* object_name) : _data(0), _handle(NULL) { _acquire(object_name); }
	~CSharedData() { _release(); }

private:

	// 初期化というかそのへん

	void _acquire(const TCHAR* object_name)
	{
		if (_handle == NULL)
			_handle = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(T), object_name);

		if (_handle != NULL)
		{
			DWORD error = ::GetLastError();

			// 領域確保
			_data = reinterpret_cast<T*>(::MapViewOfFile(_handle, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0));

			if ((_data != 0) && (error != ERROR_ALREADY_EXISTS))
				::ZeroMemory(_data, sizeof(T));	// 該当object_nameで初めて確保された領域なら0初期化
		}
	}

	// 開放

	void _release()
	{
		if (_data != 0)
		{
			::UnmapViewOfFile(_data);
			_data = 0;
		}

		if (_handle != NULL)
		{
			::CloseHandle(_handle);
			_handle = NULL;
		}

		// CreateFileMapping()は内部にカウンタ持ってるっぽいから
		// 参照カウンタが0にならない限りobject自体は削除されないみたいです
		// あれなら(GetLastError() == ERROR_ALREADY_EXISTS)の場合はFlagを立てておくとかしてくださ
	}

public:

	// インターフェース
	// 関数名でわかるだろうから略

	bool alive() { return ((_handle != NULL) && (_data != 0)); }
	T* get() { return _data; }	// 堪忍して...

};


#endif	// #ifndef SHAREDDATA_HPP
