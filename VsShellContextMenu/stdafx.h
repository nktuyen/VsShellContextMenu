// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define SUPPORTED_FILE_EXTENSION		_T("*")

// Windows Header Files
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <combaseapi.h>
#include <string>

#ifdef _UNICODE
#define	StdString	std::wstring
#else
#define StdString	std::string
#endif