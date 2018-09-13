#include "stdafx.h"
#include "VsGlobal.h"


CVsGlobal::CVsGlobal(CVsSolution* pSln)
	: CVsFileElement(EVsElement_Global)
	, m_pSolution(pSln)
{
	
}


CVsGlobal::~CVsGlobal()
{
}

bool CVsGlobal::Valid()
{
	return true;
}