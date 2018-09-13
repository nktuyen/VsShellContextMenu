#pragma once

#include "VsFileElement.h"

class CVsSolution;

class CVsGlobal : public CVsFileElement
{
	friend class CVsSolution;
	friend class CVsSolutionFile;
private:
	CVsSolution* m_pSolution;
	CVsGlobal(CVsSolution* pSln);
	virtual ~CVsGlobal();
protected:
public:
	inline const CVsSolution* const Solution() { return m_pSolution; }

	bool Valid();
};

