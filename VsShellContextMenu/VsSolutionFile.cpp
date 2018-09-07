#include "stdafx.h"
#include "VsSolutionFile.h"
#include "DebugLogger.h"
#include "VsSolution.h"
#include "VsProject.h"

#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <list>

#define PROJECT_GUIDE_LEN						38
#define VS_VERSION_SUPPORT_MIN					9.00
#define FILE_HEADER_FORMAT_STRING				StdString(_T("Microsoft Visual Studio Solution File, "))
#define FILE_HEDAER_VERSION_STRING				StdString(_T("Microsoft Visual Studio Solution File, Format Version "))
#define FILE_HEADER_VS_VERSION_STRING			StdString(_T("# Visual Studio "))
#define FILE_HEADER_PROJECT_BEGIN				StdString(_T("Project(\""))
#define FILE_HEADER_PROJECT_END					StdString(_T("EndProject"))
#define FILE_HEADER_GLOBAL_START				StdString(_T("Global"))
#define FILE_HEADER_GLOBAL_END					StdString(_T("EndGlobal"))

CVsSolutionFile::CVsSolutionFile(LPCTSTR lpszFilePath/* = nullptr*/)
	: m_pSolution(nullptr)
{
	memset(m_szFilePath, 0, MAX_PATH * sizeof(TCHAR));
	if (nullptr != lpszFilePath) {
		StringCchCopy(m_szFilePath, MAX_PATH, lpszFilePath);
	}
}


CVsSolutionFile::~CVsSolutionFile()
{
	this->Close();
}

bool CVsSolutionFile::ValidateGuidString(StdString guid)
{
	StdString temp = guid;

	if (temp.length() != PROJECT_GUIDE_LEN) {
		return false;
	}

	if (temp[0] != '{' || temp[temp.length() - 1] != '}') {
		return false;
	}

	return true;
}

bool CVsSolutionFile::Open(LPCTSTR lpszFilePath /* = nullptr */)
{
	CDebugLogger::WriteInfo(__FUNCTIONW__);
	this->Close();
	if (nullptr != lpszFilePath) {
		memset(m_szFilePath, 0, MAX_PATH * sizeof(TCHAR));
		StringCchCopy(m_szFilePath, MAX_PATH, lpszFilePath);
	}

	std::ifstream file(m_szFilePath);
	TCHAR *pszLine = nullptr;
	size_t nLineLen = 0;
	size_t nPreLen = 0;
	std::list<CVsProject*> mProjects;
	CVsProject* pCurProject = nullptr;
	bool bValid = true;
	
	try {
		std::string line;
		StdString linew;
		StdString linewSub;
		float fVersion = 0.0;
		while (std::getline(file, line)) {
			if(line.length() <= 0)
				continue;

			if(line[0] == '#')	//Comment
				continue;

			if(Trim<std::string>(line).empty())
				continue;

			pszLine = new TCHAR[line.length()+1];
			memset(pszLine, 0, (line.length() + 1)*sizeof(TCHAR));
			nLineLen = MultiByteToWideChar(CP_ACP, 0, line.c_str(), int(line.length()), pszLine, int(line.length()) * sizeof(TCHAR));
			if (nLineLen <= 0) {
				delete[] pszLine;
				pszLine = nullptr;

				break;
			}
			pszLine[nLineLen] = 0;
			linew = pszLine;
			nLineLen = linew.length();
			delete[] pszLine;
			pszLine = nullptr;
			CDebugLogger::WriteInfo(_T("linew=%s"), linew.c_str());

			if (m_pSolution == nullptr) {
				CDebugLogger::WriteInfo(_T("(m_pSolution == nullptr)"));
				if(linew == _T("﻿"))
					continue;

				nPreLen = FILE_HEADER_FORMAT_STRING.length();
				CDebugLogger::WriteInfo(_T("nLineLen=%d, FILE_HEADER_FORMAT_STRING.length()=%d"), nLineLen, nPreLen);
				if (nLineLen < nPreLen) {
					bValid = false;
					break;
				}

				linewSub = linew.substr(0, nPreLen);
				CDebugLogger::WriteInfo(_T("%d:linewSub=%s"),__LINE__, linewSub.c_str());

				if (FILE_HEADER_FORMAT_STRING.compare(linewSub) != 0) {
					CDebugLogger::WriteInfo(_T("Invalid File Format."));
					bValid = false;
					break;
				}

				nPreLen = FILE_HEDAER_VERSION_STRING.length();
				linewSub = linew.substr(0, nPreLen);
				CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());

				if (FILE_HEDAER_VERSION_STRING.compare(linewSub) != 0) {
					CDebugLogger::WriteInfo(_T("Invalid File Version."));
					bValid = false;
					break;
				}

				linewSub = linew.substr(nPreLen);
				CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());
				fVersion = float(_tstof(linewSub.c_str()));
				CDebugLogger::WriteInfo(_T("%.4f"), fVersion);
				if (fVersion <= 0.0) {
					bValid = false;
					break;
				}
				if (fVersion < VS_VERSION_SUPPORT_MIN) {
					bValid = false;
					break;
				}

				m_pSolution = new CVsSolution(fVersion);
			}
			else {
				if (FILE_HEADER_PROJECT_BEGIN.length() < linew.length()) {
					linewSub = linew.substr(0, FILE_HEADER_PROJECT_BEGIN.length());
					CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());
					if (FILE_HEADER_PROJECT_BEGIN.compare(linewSub) == 0) {
						linewSub = linew.substr(FILE_HEADER_PROJECT_BEGIN.length(), PROJECT_GUIDE_LEN);
						CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());

						if (ValidateGuidString(linewSub)) {
							if (pCurProject != nullptr) {
								delete pCurProject;
								pCurProject = nullptr;
								bValid = false;
								break;
							}

							pCurProject = new CVsProject(linewSub.c_str());
							pCurProject->Initialize();
						}
						continue;
					}
				}
				
				if (FILE_HEADER_PROJECT_END.compare(linew) == 0) {
					if (pCurProject == nullptr) {
						bValid = false;
						break;
					}

					if (pCurProject != nullptr) {
						pCurProject->Finallize();
						mProjects.push_back(pCurProject);
						pCurProject = nullptr;
					}
					continue;
				}
			}
		}
	}
	catch (...) {
		if (pszLine != nullptr) {
			delete[] pszLine;
			pszLine = nullptr;
		}

		return false;
	}

	if (m_pSolution == nullptr) {
		for (std::list<CVsProject*>::iterator ite = mProjects.begin(); ite != mProjects.end(); ++ite) {
			pCurProject = (*ite);
			delete pCurProject;
		}

		return false;
	}

	if (!bValid) {
		for (std::list<CVsProject*>::iterator ite = mProjects.begin(); ite != mProjects.end(); ++ite) {
			pCurProject = (*ite);
			delete pCurProject;
		}

		if (m_pSolution != nullptr) {
			delete m_pSolution;
			m_pSolution = nullptr;
		}

		return false;
	}

	for (std::list<CVsProject*>::iterator ite = mProjects.begin(); ite != mProjects.end(); ++ite) {
		pCurProject = (*ite);
		m_pSolution->addProject(pCurProject->m_strGuide, pCurProject);
	}

	if (!m_pSolution->Valid()) {
		delete m_pSolution;
		m_pSolution = nullptr;

		return false;
	}

	return true;
}

bool CVsSolutionFile::Valid()
{
	if (nullptr == m_pSolution)
		return false;

	return m_pSolution->Valid();
}

void CVsSolutionFile::Close()
{
	CDebugLogger::WriteInfo(__FUNCTIONW__);
	if (m_pSolution != nullptr) {
		delete m_pSolution;
		m_pSolution = nullptr;
	}
}