// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define SUPPORTED_FILE_EXTENSION				_T("*")

#define VS2005_VERSION						9						
#define VS2008_VERSION						10
#define VS2010_VERSION						11
#define VS2012_VERSION						12
#define VS2013_VERSION						13
#define VS2015_VERSION						14
#define VS2017_VERSION						15
#define VS_VERSION_SUPPORT_MIN				VS2005_VERSION
#define VS_VERSION_SUPPORT_MAX				VS2017_VERSION

// Windows Header Files
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <combaseapi.h>
#include <string>

#ifdef _UNICODE
#define StdChar				wchar_t
#define	StdString			std::wstring
#define StdStringStream		std::wistringstream
#else	
#define StdChar				char
#define StdString			std::string
#define StdStringStream		std::istringstream
#endif