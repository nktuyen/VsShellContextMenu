#include "stdafx.h"
#include "DebugLogger.h"

TCHAR CDebugLogger::m_szLogPath[MAX_PATH] = { 0 };
HANDLE CDebugLogger::m_hLogFile = INVALID_HANDLE_VALUE;
CRITICAL_SECTION CDebugLogger::m_Locker;

void CDebugLogger::SetLogPath(LPCTSTR lpszPath)
{
	InitializeCriticalSection(&m_Locker);
	memset(m_szLogPath, 0, MAX_PATH * sizeof(TCHAR));
	if (nullptr != lpszPath) {
		StringCchCopy(m_szLogPath, MAX_PATH, lpszPath);
	}
}

bool CDebugLogger::Write(LPCTSTR lpszMessage, int level /* = INFO_LEVEL */)
{
	EnterCriticalSection(&m_Locker);
	if (m_hLogFile == INVALID_HANDLE_VALUE) {
		size_t nLen = 0;
		if (!SUCCEEDED(StringCchLength(m_szLogPath, MAX_PATH, &nLen))) {
			LeaveCriticalSection(&m_Locker);
			return false;
		}

		if (0 >= nLen) {
			LeaveCriticalSection(&m_Locker);
			return false;
		}

		m_hLogFile = CreateFile(m_szLogPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, nullptr);
	}

	if (INVALID_HANDLE_VALUE == m_hLogFile) {
		LeaveCriticalSection(&m_Locker);
		return false;
	}

	SetFilePointer(m_hLogFile, 0, 0, FILE_END);

	TCHAR szLevel[10] = { 0 };
	TCHAR szLogMessage[255] = { 0 };
	SYSTEMTIME time = { 0 };
	size_t nLogSize = 0;
	DWORD dwWritenSize = 0;
	GetSystemTime(&time);
	GetLocalTime(&time);

	switch (level)
	{
	case ERROR_LEVEL:
		StringCchCopy(szLevel, 10, _T("ERROR"));
		break;
	case WARNING_LEVEL:
		StringCchCopy(szLevel, 10, _T("WARNING"));
		break;
	default:
		StringCchCopy(szLevel, 10, _T("INFO"));
		break;
	}

	HRESULT res = StringCchPrintf(szLogMessage, 255, _T("%0.4d/%0.2d/%0.2d-%0.2d:%0.2d:%0.2d.%d [%s]\t%s\r\n"),
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
		szLevel, lpszMessage);

	res = StringCchLength(szLogMessage, 255, &nLogSize);

	bool bRet = WriteFile(m_hLogFile, szLogMessage, DWORD(nLogSize * sizeof(TCHAR)), &dwWritenSize, nullptr) ? true : false;
	Close();

	LeaveCriticalSection(&m_Locker);
	return bRet;
}

bool CDebugLogger::WriteInfo(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	int nLen = _vsctprintf(lpszFormat, args) + 1;
	TCHAR* szLog = new TCHAR[nLen];
	memset(szLog, 0, nLen * sizeof(TCHAR));
	nLen = _vstprintf_s(szLog, nLen, lpszFormat, args);
	va_end(args);
	szLog[nLen] = 0;
	bool bRet = Write(szLog, INFO_LEVEL);
	delete[] szLog;
	
	return bRet;
}

bool CDebugLogger::WriteWarning(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	int nLen = _vsctprintf(lpszFormat, args) + 1;
	TCHAR* szLog = new TCHAR[nLen];
	memset(szLog, 0, nLen * sizeof(TCHAR));
	nLen = _vstprintf_s(szLog, nLen, lpszFormat, args);
	va_end(args);
	szLog[nLen] = 0;
	bool bRet = Write(szLog, WARNING_LEVEL);
	delete[] szLog;

	return bRet;
}

bool CDebugLogger::WriteError(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	int nLen = _vsctprintf(lpszFormat, args) + 1;
	TCHAR* szLog = new TCHAR[nLen];
	memset(szLog, 0, nLen * sizeof(TCHAR));
	nLen = _vstprintf_s(szLog, nLen, lpszFormat, args);
	va_end(args);
	szLog[nLen] = 0;
	bool bRet = Write(szLog, ERROR_LEVEL);
	delete[] szLog;

	return bRet;
}

void CDebugLogger::Close()
{
	if (m_hLogFile != INVALID_HANDLE_VALUE) {
		if (CloseHandle(m_hLogFile))
			m_hLogFile = INVALID_HANDLE_VALUE;
	}
}