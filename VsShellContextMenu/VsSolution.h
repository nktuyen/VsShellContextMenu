#pragma once

#include "VsFileElement.h"

#include <map>

class CVsProject;
class CVsGlobal;

class CVsSolution : public CVsFileElement
{
	friend class CVsSolutionFile;
private:
	struct VersionInfo{
		unsigned char Major;
		unsigned char Minor;
	} m_Version;

	std::map<StdString, CVsProject*> m_Projects;
	std::map<StdString, CVsProject*>::iterator m_Ite;
	std::map<StdString, CVsProject*>::const_iterator m_CIte;
	StdString m_strPath;
	CVsGlobal* m_pGlobal;

	CVsSolution(LPCTSTR lpszPath, float fVer = 0.0);
	virtual ~CVsSolution();
protected:
	bool addProject(StdString guid, CVsProject* pProject);
	void setVersion(float fVer);
public:
	bool Valid();
	inline const VersionInfo& Version() { return m_Version; }
	inline const LPCTSTR Path() { return m_strPath.c_str(); }
	LPCTSTR Name();
	inline CVsGlobal* const Global() { return m_pGlobal; }

	CVsProject* firstProject();
	CVsProject* nextProject();
};

