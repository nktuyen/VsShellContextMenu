#pragma once

#include <map>

class CVsSolution;

template <typename S>
S Trim(const S& input) {
	S output = input;
	while (output.length() > 0 && output[0] == ' ') { output.erase(0); }
	while (output.length() > 0 && output[output.length() - 1] == ' ') { output.erase(output.length() - 1); }

	return output;
}


typedef struct ProjectMapKeyCompare
{
	bool operator()(LPCTSTR lpszKey1, LPCTSTR lpszKey2) {
		return _tcscmp(lpszKey1, lpszKey2);
	}
}ProjectMapKeyCompare;

class CVsSolutionFile
{
public:
	CVsSolutionFile(LPCTSTR lpszFilePath = nullptr);
	virtual ~CVsSolutionFile();
	bool Open(LPCTSTR lpszFilePath  = nullptr);
	void Close();

	bool Valid();
	inline CVsSolution* Solution() const { return m_pSolution; }
protected:
private:
	TCHAR m_szFilePath[MAX_PATH];
	CVsSolution* m_pSolution;

	bool ValidateGuidString(StdString guid);
};

