#include "stdafx.h"
#include "VsSolutionFile.h"
#include "DebugLogger.h"
#include "VsSolution.h"
#include "VsProject.h"
#include "VsGlobal.h"

#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <list>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>

#define PROJECT_GUIDE_LEN						38
#define FILE_HEADER_FORMAT_STRING				StdString(_T("Microsoft Visual Studio Solution File, "))
#define FILE_HEDAER_VERSION_STRING				StdString(_T("Microsoft Visual Studio Solution File, Format Version "))
#define FILE_HEADER_VS_VERSION_REMARK_STRING	StdString(_T("# Visual Studio "))
#define FILE_HEADER_VS_VERSION_STRING			StdString(_T("VisualStudioVersion"))
#define FILE_HEADER_PROJECT_BEGIN				StdString(_T("Project(\""))
#define FILE_HEADER_PROJECT_END					StdString(_T("EndProject"))
#define FILE_HEADER_GLOBAL_START				StdString(_T("Global"))
#define FILE_HEADER_GLOBAL_END					StdString(_T("EndGlobal"))
#define FILE_HEADER_GLOBALSECTION_START			StdString(_T("GlobalSection"))
#define FILE_HEADER_GLOBALSECTION_END			StdString(_T("EndGlobalSection"))


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

std::vector<StdString> CVsSolutionFile::SplitString(const StdString& input, StdChar c)
{
	std::vector<StdString> out;
	size_t nStart = 0;
	StdString strTemp;
	size_t nPos = input.find(c, nStart);

	while ( (nPos != std::string::npos) && (nStart < input.length()) ) {
		strTemp = input.substr(nStart, nPos - nStart);
		out.push_back(strTemp);
		nStart = (nPos + 1);
		nPos = input.find(c, nStart);
	}

	if (nStart < input.length()) {
		strTemp = input.substr(nStart);
		out.push_back(strTemp);
	}
	
	return out;
}

bool CVsSolutionFile::ValidateGuidString(StdString guid)
{
	StdString temp = guid;

	if (temp.length() != PROJECT_GUIDE_LEN) {
		CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
		return false;
	}

	if (temp[0] != '{' || temp[temp.length() - 1] != '}') {
		CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
		return false;
	}

	return true;
}

EVsSolutionFileError CVsSolutionFile::Open(LPCTSTR lpszFilePath /* = nullptr */)
{
	EVsSolutionFileError eRet = EVsSolutionFileError_NoErr;
	//CDebugLogger::WriteInfo(__FUNCTIONW__);
	this->Close();
	if (nullptr != lpszFilePath) {
		memset(m_szFilePath, 0, MAX_PATH * sizeof(TCHAR));
		StringCchCopy(m_szFilePath, MAX_PATH, lpszFilePath);
	}

	std::ifstream file(m_szFilePath);
	std::vector<StdString> arrString;
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
		float fFileVersion = 0.0;
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
				CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
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
					CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
					break;
				}

				linewSub = linew.substr(nPos, nPreLen);

				if (FILE_HEADER_FORMAT_STRING.compare(linewSub) != 0) {
					CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
					eRet = EVsSolutionFileError_Invalid;
					break;
				}

				nPreLen = FILE_HEDAER_VERSION_STRING.length();
				linewSub = linew.substr(nPos, nPreLen);

				if (FILE_HEDAER_VERSION_STRING.compare(linewSub) != 0) {
					CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
					eRet = EVsSolutionFileError_Invalid;
					break;
				}
				nPos += FILE_HEDAER_VERSION_STRING.length();

				linewSub = linew.substr(nPos);
				fFileVersion = float(_tstof(linewSub.c_str()));

				if (fFileVersion <= 0.0) {
					eRet = EVsSolutionFileError_Invalid;
					CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
					break;
				}

				if (fFileVersion < VS_VERSION_SUPPORT_MIN) {
					CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
					eRet = EVsSolutionFileError_UnSupported;
					break;
				}

				if (fFileVersion > VS_VERSION_SUPPORT_MAX) {
					CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
					eRet = EVsSolutionFileError_UnSupported;
					break;
				}

				m_pSolution = new CVsSolution(lpszFilePath, fFileVersion);
				m_pSolution->Initialize();
			}
			else {
				nPos = 0;
				if (FILE_HEADER_VS_VERSION_STRING.length() < linew.length()) {
					linewSub = linew.substr(nPos, FILE_HEADER_VS_VERSION_STRING.length());
					nPos += FILE_HEADER_VS_VERSION_STRING.length();
					if (FILE_HEADER_VS_VERSION_STRING.compare(linewSub) == 0) {
						arrString = SplitString(linew,'=');
						//CDebugLogger::WriteInfo(_T("%s[%d] %d"), __FUNCTIONW__, __LINE__, arrString.size());
						if (arrString.size() == 2) {
							linewSub = Trim<StdString>(arrString[0]);
							if (linewSub.compare(FILE_HEADER_VS_VERSION_STRING) == 0) {
								StdString strVsVersion = Trim<StdString>(arrString[1]);
								float fVsVersion= float(_tstof(strVsVersion.c_str()));
								if (fVsVersion <= 0.0f) {
									CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
									eRet = EVsSolutionFileError_UnSupported;
									break;
								}

								if (nullptr == m_pSolution) {
									CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
									eRet = EVsSolutionFileError_UnSupported;
									break;
								}

								m_pSolution->setVersion(fVsVersion);
								continue;
							}
						}
					}
				}

				nPos = 0;
				if (FILE_HEADER_PROJECT_BEGIN.length() < linew.length()) {
					linewSub = linew.substr(nPos, FILE_HEADER_PROJECT_BEGIN.length());
					//CDebugLogger::WriteInfo(_T("%s[%d] %s"), __FUNCTIONW__, __LINE__, linewSub.c_str());
					if (FILE_HEADER_PROJECT_BEGIN.compare(linewSub) == 0) {
						//CDebugLogger::WriteInfo(_T("%s[%d] %s"), __FUNCTIONW__, __LINE__, linewSub.c_str());
						nPos += FILE_HEADER_PROJECT_BEGIN.length();
						linewSub = linew.substr(nPos, PROJECT_GUIDE_LEN);	//Project Guid
						//CDebugLogger::WriteInfo(_T("%s[%d] %s"), __FUNCTIONW__, __LINE__, linewSub.c_str());
						if (ValidateGuidString(linewSub)) {
							if (pCurProject != nullptr) {
								delete pCurProject;
								pCurProject = nullptr;
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}
							
							pCurProject = new CVsProject(linewSub.c_str());
							pCurProject->Initialize();

							nPos += linewSub.length() + 1;
							linewSub = linew.substr(nPos+1);	//Project name
							nPos += TrimLeft(linewSub);

							if (linewSub.empty()) {
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}

							nPosSub = linewSub.find('=');
							if (nPos == StdString::npos) {
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}

							nPos += nPosSub + 1;
							linewSub = linew.substr(nPos + 1);
							nPos += TrimLeft(linewSub);
							
							arrString = SplitString(linewSub, ',');
							
							if (arrString.size() <= 0) {
								eRet = EVsSolutionFileError_Invalid;
								CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
								break;
							}

							nPos = 0;
							std::for_each(arrString.begin(), arrString.end(), [&](StdString& s) {

								linewSub = Trim(arrString[nPos]);
								nPos++;

								if (linewSub.empty()) {
									eRet = EVsSolutionFileError_Invalid;
									CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
									return;
								}

								if (linewSub[0] == '\"') {
									if (linewSub[linewSub.length() - 1] != '\"') {
										eRet = EVsSolutionFileError_Invalid;
										CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
										return;
									}

									linewSub.erase(0,1);
									linewSub.erase(linewSub.length() - 1, 1);
								}

								if (nPos == 1) {
									pCurProject->m_strName = linewSub;
								}
								else if (nPos == 2) {
									pCurProject->m_strPath = linewSub;
								}
								else if (nPos == 3) {
									if (!ValidateGuidString(linewSub)) {
										eRet = EVsSolutionFileError_Invalid;
										CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
										return;
									}

									pCurProject->m_strId = linewSub;
								}
							});
						}

						if (eRet != EVsSolutionFileError_NoErr)
							break;
						else
							continue;
					}
				}
				
				nPos = 0;
				if (FILE_HEADER_PROJECT_END.compare(linew) == 0) {
					if (pCurProject == nullptr) {
						eRet = EVsSolutionFileError_Invalid;
						CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
						break;
					}

					pCurProject->Finallize();
					mProjects.push_back(pCurProject);
					pCurProject = nullptr;

					continue;
				}

				nPos = 0;
				if (FILE_HEADER_GLOBAL_START.compare(linew) == 0) {
					if ((nullptr == m_pSolution) || (nullptr == m_pSolution->Global())) {
						eRet = EVsSolutionFileError_Invalid;
						CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
						break;
					}

					m_pSolution->Global()->Initialize();

					continue;
				}

				nPos = 0;
				if (FILE_HEADER_GLOBAL_END.compare(linew) == 0) {
					if ((nullptr == m_pSolution) || (nullptr == m_pSolution->Global())) {
						eRet = EVsSolutionFileError_Invalid;
						CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
						break;
					}

					m_pSolution->Global()->Finallize();

					continue;
				}

				continue;
			}
		}
	}
	catch (...) {
		if (pszLine != nullptr) {
			delete[] pszLine;
			pszLine = nullptr;
		}
		CDebugLogger::WriteError(_T("%s[%d] Exception"), __FUNCTIONW__, __LINE__);
		return eRet;
	}


	if (pCurProject != nullptr) {
		delete pCurProject;
		eRet = EVsSolutionFileError_Invalid;
		CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
	}

	if (m_pSolution == nullptr) {
		eRet = EVsSolutionFileError_Invalid;
		CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
	}

	if (eRet != EVsSolutionFileError_NoErr) {
		CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
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

	for (std::list<CVsProject*>::iterator ite = mProjects.begin(); ite != mProjects.end(); ++ite) {
		pCurProject = (*ite);
		if (pCurProject != nullptr) {
			m_pSolution->addProject(pCurProject->m_strId, pCurProject);
		}
	}

	m_pSolution->Finallize();

	if (!m_pSolution->Valid()) {
		delete m_pSolution;
		m_pSolution = nullptr;

		eRet = EVsSolutionFileError_Invalid;
		CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
		return eRet;
	}

	return eRet;
}

bool CVsSolutionFile::Valid()
{
	if (nullptr == m_pSolution) {
		CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
		return false;
	}

	return m_pSolution->Valid();
}

void CVsSolutionFile::Close()
{
	if (m_pSolution != nullptr) {
		delete m_pSolution;
		m_pSolution = nullptr;
	}
}