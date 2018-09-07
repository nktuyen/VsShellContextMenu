#pragma once

#include "VsFileElement.h"

#include <map>

class CVsProject;

class CVsSolution : public CVsFileElement
{
	friend class CVsSolutionFile;
private:
	struct VersionInfo{
		unsigned char Major;
		unsigned char Minor;
	} m_Version;

	std::map<StdString, CVsProject*> m_Projects;

	CVsSolution(float fVer = 0.0);
	virtual ~CVsSolution();
protected:
	bool addProject(StdString guid, CVsProject* pProject);
public:
	bool Valid();
	inline const VersionInfo& Version() { return m_Version; }
};

