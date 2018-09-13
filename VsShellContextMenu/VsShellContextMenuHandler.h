#pragma once

#include <shlobj.h>
#include "VsSolutionFile.h"

class CVsShellContextMenuHandler : public IShellExtInit, public IContextMenu
{
public:
	CVsShellContextMenuHandler(void);
	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	// IShellExtInit
	IFACEMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID);

	// IContextMenu
	IFACEMETHODIMP QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
	IFACEMETHODIMP GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax);
protected:
	virtual ~CVsShellContextMenuHandler(void);
private:
	PCWSTR m_pszMenuText;
	HANDLE m_hSettingsMenuBmp;
	std::map<UINT, HANDLE> m_hMainBitmaps;
	PCSTR m_pszVerb;
	PCWSTR m_pwszVerb;
	PCSTR m_pszVerbCanonicalName;
	PCWSTR m_pwszVerbCanonicalName;
	PCSTR m_pszVerbHelpText;
	PCWSTR m_pwszVerbHelpText;
	CVsSolutionFile m_File;
	// Reference count of component.
	long m_cRef;

	// The name of the selected file.
	wchar_t m_szSelectedFile[MAX_PATH];

	// The method that handles the "display" verb.
	void OnVerbDisplayFileName(HWND hWnd);
};

