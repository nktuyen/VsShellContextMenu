#include "stdafx.h"
#include "VsSolution.h"


CVsSolution::CVsSolution(float fVer /* = 0.0 */)
	: CVsFileElement(EVsElement_Solution)
{
	m_Version.Major = 0;
	m_Version.Minor = 0;
}


CVsSolution::~CVsSolution()
{
}

bool CVsSolution::Valid()
{
	if (m_Version.Major < 9)
		return false;

	return true;
}

bool CVsSolution::addProject(StdString guid, CVsProject* pProject)
{
	if (guid.empty() || pProject == nullptr)
		return false;

	m_Projects[guid] = pProject;
	return true;
}