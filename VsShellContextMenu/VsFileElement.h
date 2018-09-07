#pragma once

typedef enum EVsElement_ {
	EVsElement_Unknown = 0,
	EVsElement_Solution,
	EVsElement_Project,
	EVsElement_ProjectSection,
	EVsElement_Global,
	EVsElement_GlobalSection,
}EVsElement;

class CVsFileElement
{
public:
	inline EVsElement getKind() { return m_eKind; }
protected:
	CVsFileElement(const EVsElement eElementType);
	virtual ~CVsFileElement();
	virtual void Initialize() { ; }
	virtual void Finallize() { ; }
	virtual bool Valid() { return false; }
protected:
	EVsElement m_eKind;
};

