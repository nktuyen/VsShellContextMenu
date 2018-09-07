#pragma once

#include "VsFileElement.h"

class CVsProject : public CVsFileElement
{
	friend class CVsSolution;
	friend class CVsSolutionFile;
private:
	CVsProject(LPCTSTR lpszGuide = nullptr);
	virtual ~CVsProject();


	StdString m_strGuide;
	StdString m_strName;
};

