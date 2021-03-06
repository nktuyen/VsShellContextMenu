#include "stdafx.h"
#include "VsSolution.h"
#include "VsProject.h"
#include "VsGlobal.h"
#include "DebugLogger.h"

#include <algorithm>

CVsSolution::CVsSolution(LPCTSTR lpszPath, float fVer /* = 0.0 */)
	: CVsFileElement(EVsElement_Solution)
{
	double fMajor = 0.0, fMinor = 0.0;
	fMinor = modf(fVer, &fMajor);
	m_Version.Minor = fMinor;
	m_Version.Major = fMajor;
	m_Ite = m_Projects.end();
	m_CIte = m_Projects.cend();
	if (nullptr != lpszPath) {
		m_strPath = StdString(lpszPath);
	}

	m_pGlobal = new CVsGlobal(this);
}


CVsSolution::~CVsSolution()
{
	if (nullptr != m_pGlobal) {
		delete m_pGlobal;
		m_pGlobal = nullptr;
	}
}

LPCTSTR CVsSolution::Name()
{
	StdString path = this->Path();
	size_t nStart = path.find_last_of('\\');
	if (nStart == StdString::npos)
		nStart = -1;

	return path.substr(nStart + 1).c_str();
}

void CVsSolution::setVersion(float fVer)
{
	double fMajor = 0.0, fMinor = 0.0;
	fMinor = modf(fVer, &fMajor);
	m_Version.Minor = fMinor;
	m_Version.Major = fMajor;
}

bool CVsSolution::Valid()
{
	if (!m_pGlobal->Valid()) {
		CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
		return false;
	}

	bool bValid = true;
	std::for_each(m_Projects.begin(), m_Projects.end(), [&](std::pair<StdString, CVsProject*> ite) {
		CVsProject* prj = ite.second;
		if (nullptr != prj) {
			if (!prj->Valid()) {
				bValid = false;
				CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
				return;
			}
		}
	});

	return bValid;
}

bool CVsSolution::addProject(StdString guid, CVsProject* pProject)
{
	if (guid.empty() || pProject == nullptr) {
		CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
		return false;
	}

	m_Projects.insert(std::make_pair(guid,pProject));
	CDebugLogger::WriteInfo(_T("%s[%d], m_Projects.size()=%d"), __FUNCTIONW__, __LINE__, m_Projects.size());

	return true;
}

CVsProject* CVsSolution::firstProject()
{
	m_Ite = m_Projects.begin();

	if (m_Ite != m_Projects.end())
		return m_Ite->second;
	else
		return nullptr;
}


CVsProject* CVsSolution::nextProject()
{
	++m_Ite;
	
	if (m_Ite != m_Projects.end())
		return m_Ite->second;
	else
		return nullptr;
}
