#pragma once

#include <map>
#include "DebugLogger.h"

class CVsSolution;

template <typename S>
S Trim(const S& input) {
	size_t start = 0, end = input.length();
	while (input[start]==' '){start++;}
	while (end > start && input[end - 1] == ' ') { end--; }

	return input.substr(start, end - start + 1);
}

template <typename S>
size_t TrimLeft(S& input) {
	size_t start = 0, end = input.length();
	while (input[start] == ' ') { start++; }

	input= input.substr(start, end - start + 1);
	return start;
}

typedef struct ProjectMapKeyCompare
{
	bool operator()(LPCTSTR lpszKey1, LPCTSTR lpszKey2) {
		return _tcscmp(lpszKey1, lpszKey2);
	}
}ProjectMapKeyCompare;

typedef enum EVsSolutionFileError {
	EVsSolutionFileError_NoErr = 0,
	EVsSolutionFileError_Invalid,
	EVsSolutionFileError_UnSupported
}EVsSolutionFileError;

class CVsSolutionFile
{
public:
	CVsSolutionFile(LPCTSTR lpszFilePath = nullptr);
	virtual ~CVsSolutionFile();
	EVsSolutionFileError Open(LPCTSTR lpszFilePath  = nullptr);
	void Close();

	bool Valid();
	inline CVsSolution* Solution() const { return m_pSolution; }
protected:
private:
	TCHAR m_szFilePath[MAX_PATH];
	CVsSolution* m_pSolution;

	bool ValidateGuidString(StdString guid);
};

