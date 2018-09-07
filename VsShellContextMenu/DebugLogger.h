#pragma once
class CDebugLogger
{
public:
	enum {
		NONE = 0,
		INFO_LEVEL,
		WARNING_LEVEL,
		ERROR_LEVEL
	};
	static void SetLogPath(LPCTSTR lpszPath);
	static bool WriteInfo(LPCTSTR lpszFormat,...);
	static bool WriteWarning(LPCTSTR lpszFormat, ...);
	static bool WriteError(LPCTSTR lpszFormat, ...);
	static void Close();
private:
	static CRITICAL_SECTION m_Locker;
	static HANDLE m_hLogFile;
	static TCHAR m_szLogPath[MAX_PATH];

	static bool Write(LPCTSTR lpszMessage, int level = NONE);
};

