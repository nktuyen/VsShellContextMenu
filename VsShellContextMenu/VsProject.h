#pragma once

#include "VsFileElement.h"

class CVsProject : public CVsFileElement
{
	friend class CVsSolution;
	friend class CVsSolutionFile;
private:
	CVsProject(LPCTSTR lpszGuide = nullptr, LPCTSTR lpszName = nullptr);
	virtual ~CVsProject();


	StdString m_strGuide;
	StdString m_strName;
protected:
public:
	LPCTSTR Guid() { return m_strGuide.c_str(); }
	LPCTSTR Name() { return m_strName.c_str(); }
};

