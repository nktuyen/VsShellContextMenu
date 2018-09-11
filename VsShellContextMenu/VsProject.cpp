#include "stdafx.h"
#include "VsProject.h"
#include "DebugLogger.h"


CVsProject::CVsProject(LPCTSTR lpszGuide /* = nullptr */, LPCTSTR lpszName /* = nullptr */)
	:CVsFileElement(EVsElement_Project)
{
	if (lpszGuide != nullptr)
		m_strGuide = lpszGuide;

	if (lpszName != nullptr)
		m_strName = lpszName;
}


CVsProject::~CVsProject()
{
}
