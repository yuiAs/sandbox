#ifndef COMMON_H_E98D0960_9AA4_4ccb_9C31_F10067C2CFC7
#define COMMON_H_E98D0960_9AA4_4ccb_9C31_F10067C2CFC7

#include <winsock2.h>
#include <windows.h>
#include "vc2005.h"
#include "dbgprint.h"

// types

#ifndef _WINSOCK2API_
/*
 * Basic system type definitions, taken from the BSD file sys/types.h.
 */
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
typedef unsigned __int64 u_int64;
#endif	// #ifndef _WINSOCK2API_

#ifndef _STDINT_H
typedef signed char		int8_t;
typedef signed short	int16_t;
typedef signed long		int32_t;
typedef signed __int64	int64_t;
typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned long		uint32_t;
typedef unsigned __int64	uint64_t;
#endif	// #ifndef _STDINT_H

// macro

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete [] (p); (p)=NULL; } }
#endif

// function

template<typename T> T& _cast(const void* p)
{
	return *reinterpret_cast<T*>(p);
}

static inline uint32_t _mkU32(void* p)
{
	return *reinterpret_cast<uint32_t*>(p);
}

static inline uint32_t _mkU32(uint32_t n)
{
	return _mkU32(reinterpret_cast<void*>(n));
}

static inline uint16_t _mkU16(void* p)
{
	return *reinterpret_cast<uint16_t*>(p);
}

static inline uint8_t _mkU8(void* p)
{
	return *reinterpret_cast<uint8_t*>(p);
}


#endif	// #ifndef COMMON_H
