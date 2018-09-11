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
#define VS_VERSION_SUPPORT_MAX					12.00
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
	CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
	StdString temp = guid;

	if (temp.length() != PROJECT_GUIDE_LEN) {
		CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
		return false;
	}

	if (temp[0] != '{' || temp[temp.length() - 1] != '}') {
		CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
		return false;
	}

	CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
	return true;
}

EVsSolutionFileError CVsSolutionFile::Open(LPCTSTR lpszFilePath /* = nullptr */)
{
	EVsSolutionFileError eRet = EVsSolutionFileError_NoErr;
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
	size_t nPos = -1;
	size_t nPosSub = -1;
	std::list<CVsProject*> mProjects;
	CVsProject* pCurProject = nullptr;
	
	try {
		std::string line;
		StdString linew;
		StdString linewSub;
		float fVersion = 0.0;
		while (std::getline(file, line)) {
			line = Trim<std::string>(line);

			if(line.length() <= 0)
				continue;

			if(line[0] == '#')	//Comment
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
			linew = Trim<StdString>(pszLine);
			if(linew.empty())
				continue;
			nPos = 0;

			nLineLen = linew.length();
			delete[] pszLine;
			pszLine = nullptr;

			if (m_pSolution == nullptr) {
				if (linew == _T("﻿")) {
					continue;
				}

				nPreLen = FILE_HEADER_FORMAT_STRING.length();
				if (nLineLen < nPreLen) {
					eRet = EVsSolutionFileError_Invalid;
					break;
				}

				linewSub = linew.substr(nPos, nPreLen);
				CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);

				if (FILE_HEADER_FORMAT_STRING.compare(linewSub) != 0) {
					CDebugLogger::WriteInfo(_T("Invalid File Format."));
					eRet = EVsSolutionFileError_Invalid;
					break;
				}

				nPreLen = FILE_HEDAER_VERSION_STRING.length();
				linewSub = linew.substr(nPos, nPreLen);
				CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());

				if (FILE_HEDAER_VERSION_STRING.compare(linewSub) != 0) {
					CDebugLogger::WriteInfo(_T("Invalid File Version."));
					eRet = EVsSolutionFileError_Invalid;
					break;
				}
				nPos += FILE_HEDAER_VERSION_STRING.length();

				linewSub = linew.substr(nPos);
				CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());
				fVersion = float(_tstof(linewSub.c_str()));
				CDebugLogger::WriteInfo(_T("%.4f"), fVersion);
				if (fVersion <= 0.0) {
					eRet = EVsSolutionFileError_Invalid;
					break;
				}

				if (fVersion < VS_VERSION_SUPPORT_MIN) {
					eRet = EVsSolutionFileError_UnSupported;
					break;
				}

				if (fVersion > VS_VERSION_SUPPORT_MAX) {
					eRet = EVsSolutionFileError_UnSupported;
					break;
				}

				m_pSolution = new CVsSolution(fVersion);
				m_pSolution->Initialize();
				CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
			}
			else {
				CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
				if (FILE_HEADER_PROJECT_BEGIN.length() < linew.length()) {
					linewSub = linew.substr(nPos, FILE_HEADER_PROJECT_BEGIN.length());
					CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());
					if (FILE_HEADER_PROJECT_BEGIN.compare(linewSub) == 0) {
						nPos += FILE_HEADER_PROJECT_BEGIN.length();
						linewSub = linew.substr(nPos, PROJECT_GUIDE_LEN);	//Project Guid
						CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());

						if (ValidateGuidString(linewSub)) {
							if (pCurProject != nullptr) {
								delete pCurProject;
								pCurProject = nullptr;
								eRet = EVsSolutionFileError_Invalid;
								break;
							}
							
							CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
							pCurProject = new CVsProject(linewSub.c_str());
							pCurProject->Initialize();

							nPos += linewSub.length() + 1;
							linewSub = linew.substr(nPos+1);	//Project name
							nPos += TrimLeft(linewSub);
							CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());

							if (linewSub.empty()) {
								eRet = EVsSolutionFileError_Invalid;
								break;
							}

							nPosSub = linewSub.find('=');
							if (nPos < 0) {
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}

							nPos += nPosSub + 1;
							linewSub = linew.substr(nPos + 1);
							nPos += TrimLeft(linewSub);
							CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());
							if (linewSub.empty()) {
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}

							if (linewSub[0] != '\"') {
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}
							nPos += 1;

							nPosSub = linewSub.find('\"', 1);
							if(nPosSub < 0) {
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}

							linewSub = linew.substr(nPos, nPosSub + 1);	//Project name
							CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());
							nPos += TrimLeft(linewSub);
							CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());

							if (linewSub.empty()) {
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}

							if (linewSub[0] == '\"') {
								if (linewSub[linewSub.length() - 1] != '\"') {
									eRet = EVsSolutionFileError_Invalid;
									CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
									break;
								}

								pCurProject->m_strName = linewSub.substr(1, linewSub.length()-1-1);
							}
							else {
								pCurProject->m_strName = linewSub;
							}
							
							CDebugLogger::WriteInfo(_T("%d:pCurProject->m_strName=%s"), __LINE__, pCurProject->m_strName.c_str());

							nPos += linewSub.length();

							linewSub = linew.substr(nPos);
							CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());
							nPos += TrimLeft(linewSub);

							if (linewSub[0] != ',') {
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}

							nPos += 1;
							linewSub = linew.substr(nPos);
							nPos += TrimLeft(linewSub);
							CDebugLogger::WriteInfo(_T("%d:linewSub=%s"), __LINE__, linewSub.c_str());

							continue;
						}
						
						eRet = EVsSolutionFileError_Invalid;
						break;
					}
				}
				
				CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
				if (FILE_HEADER_PROJECT_END.compare(linew) == 0) {
					if (pCurProject == nullptr) {
						eRet = EVsSolutionFileError_Invalid;
						CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
						break;
					}

					if (pCurProject != nullptr) {
						pCurProject->Finallize();
						mProjects.push_back(pCurProject);
						pCurProject = nullptr;
						CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
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
		CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
		return eRet;
	}

	CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);

	if (pCurProject != nullptr) {
		delete pCurProject;
		eRet = EVsSolutionFileError_Invalid;
	}

	CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
	if (m_pSolution == nullptr) {
		eRet = EVsSolutionFileError_Invalid;
	}

	CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
	if (eRet != EVsSolutionFileError_NoErr) {
		for (std::list<CVsProject*>::iterator ite = mProjects.begin(); ite != mProjects.end(); ++ite) {
			pCurProject = (*ite);
			delete pCurProject;
		}
		mProjects.clear();

		if (m_pSolution != nullptr) {
			delete m_pSolution;
			m_pSolution = nullptr;
		}

		return eRet;
	}

	CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
	for (std::list<CVsProject*>::iterator ite = mProjects.begin(); ite != mProjects.end(); ++ite) {
		pCurProject = (*ite);
		if (pCurProject != nullptr) {
			m_pSolution->addProject(pCurProject->m_strGuide, pCurProject);
		}
	}

	m_pSolution->Finallize();
	CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);

	if (!m_pSolution->Valid()) {
		delete m_pSolution;
		m_pSolution = nullptr;

		eRet = EVsSolutionFileError_Invalid;
		return eRet;
	}
	CDebugLogger::WriteInfo(_T("%s[%d]"), __FUNCTIONW__, __LINE__);

	return eRet;
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