//! @file   stdafx.h
//! @date   2008/11/20  1st
#pragma once

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500)
#define _WIN32_WINNT 0x0501
#endif

// This function or variable may be unsafe.
#pragma warning(disable: 4996)

#include <windows.h>
#include <process.h>
#include <stdio.h>
#if defined(_UNICODE)
#include <wchar.h>
#endif

#include "shared/linklist.h"
#include "shared/gdiplus.h"
#include "shared/util.h"
#include "shared/debug.h"
