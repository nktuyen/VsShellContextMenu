#include "stdafx.h"
#include "VsProject.h"


CVsProject::CVsProject(LPCTSTR lpszGuide /* = nullptr */)
	:CVsFileElement(EVsElement_Project)
	, m_strGuide(lpszGuide)
{
}


CVsProject::~CVsProject()
{
}
