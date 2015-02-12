// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#define WIN32_LEAN_AND_MEAN
#define _WIN32_DCOM

#include "targetver.h"

#include <windows.h>
#include <tchar.h>
#include <comdef.h>


#pragma warning(push)
#pragma warning(disable: 4192)
#import <msxml3.dll> raw_interfaces_only
#import <shdocvw.dll>
#import <mshtml.tlb> /*raw_interfaces_only*/ auto_rename
#pragma warning(pop)


#include "xmlconf.h"
