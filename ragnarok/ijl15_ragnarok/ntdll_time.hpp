#ifndef NTDLL_TIME_H
#define NTDLL_TIME_H

#include <windows.h>



#if !defined(_NTDEF_) || !defined(_WINNT_)

// API

#define NTAPI __stdcall

typedef LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;

// TIME_FILELDS

typedef short CSHORT;

typedef struct _TIME_FIELDS {
    CSHORT Year;        // range [1601...]
    CSHORT Month;       // range [1..12]
    CSHORT Day;         // range [1..31]
    CSHORT Hour;        // range [0..23]
    CSHORT Minute;      // range [0..59]
    CSHORT Second;      // range [0..59]
    CSHORT Milliseconds;// range [0..999]
    CSHORT Weekday;     // range [0..6] == [Sunday..Saturday]
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;

#endif	// #if !defined(_NTDEF_) || !defined(_WINNT_)

////////////////////////////////////////////////////////////////////////////////


typedef NTSTATUS (NTAPI *NT_NTQUERYSYSTIME)(PLARGE_INTEGER);
typedef NTSTATUS (NTAPI *NT_RTLSYSTMTOLOCALTM)(PLARGE_INTEGER, PLARGE_INTEGER);
typedef BOOLEAN (NTAPI *NT_RTLTMTOSECONDS)(PLARGE_INTEGER, PULONG);
typedef VOID (NTAPI *NT_RTLSECONDSTOTM)(ULONG, PLARGE_INTEGER);
typedef BOOLEAN (NTAPI *NT_RTLTFTOTM)(PTIME_FIELDS, PLARGE_INTEGER);
typedef VOID (NTAPI *NT_RTLTMTOTF)(PLARGE_INTEGER, PTIME_FIELDS);

static NT_NTQUERYSYSTIME NtQuerySystemTime;
static NT_RTLSYSTMTOLOCALTM RtlSystemTimeToLocalTime;
static NT_RTLTMTOSECONDS RtlTimeToSecondsSince1970;
static NT_RTLSECONDSTOTM RtlSecondsSince1970ToTime;
static NT_RTLTFTOTM RtlTimeFieldsToTime;
static NT_RTLTMTOTF RtlTimeToTimeFields;

////////////////////////////////////////////////////////////////////////////////


namespace NTDLL
{

template <typename T>
inline T __addr(const char* m, const char* f) { return reinterpret_cast<T>(::GetProcAddress(::GetModuleHandle(m), f)); }


static void NTDLLInitialize()
{
	NtQuerySystemTime = __addr<NT_NTQUERYSYSTIME>("NTDLL", "NtQuerySystemTime");
	RtlSystemTimeToLocalTime = __addr<NT_RTLSYSTMTOLOCALTM>("NTDLL", "RtlSystemTimeToLocalTime");
	RtlTimeToSecondsSince1970 = __addr<NT_RTLTMTOSECONDS>("NTDLL", "RtlTimeToSecondsSince1970");
	RtlSecondsSince1970ToTime = __addr<NT_RTLSECONDSTOTM>("NTDLL", "RtlSecondsSince1970ToTime");
	RtlTimeFieldsToTime = __addr<NT_RTLTFTOTM>("NTDLL", "RtlTimeFieldsToTime");
	RtlTimeToTimeFields = __addr<NT_RTLTMTOTF>("NTDLL", "RtlTimeToTimeFields");
}

};	// namespace NTDLL



#endif	/// #ifndef NTDLL_TIME_H
