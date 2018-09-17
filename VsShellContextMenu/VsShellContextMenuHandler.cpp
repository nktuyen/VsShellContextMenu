#include "stdafx.h"
#include "resource.h"
#include "VsShellContextMenuHandler.h"
#include "DebugLogger.h"
#include "VsSolution.h"
#include "VsProject.h"

#include <strsafe.h>
#include <Shlwapi.h>
#include <shellapi.h>

#pragma comment(lib, "shlwapi.lib")

#define IDM_DISPLAY		0
#define IDM_SEPERATOR	1
#define IDM_SETTINGS	2

extern HINSTANCE g_hInst;
extern long g_cDllRef;

CVsShellContextMenuHandler::CVsShellContextMenuHandler(void)
	: m_cRef(1),
	m_pwszVerb(L"slncmd"),
	m_pszVerb("slncmd"),
	m_pszVerbCanonicalName("VsSolutionCommand"),
	m_pwszVerbCanonicalName(L"VsSolutionCommand"),
	m_pszVerbHelpText("Perform Visual Studio Command On Selected Solution"),
	m_pwszVerbHelpText(L"Perform Visual Studio Command On Selected Solution")
{
	InterlockedIncrement(&g_cDllRef);

	// Load the bitmap for the menu item. 
	// If you want the menu item bitmap to be transparent, the color depth of 
	// the bitmap must not be greater than 8bpp.
	m_hSettingsMenuBmp = LoadImage(g_hInst, MAKEINTRESOURCE(IDB_VsShellContextMenu), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
	for (UINT nIndex = VS2005_VERSION; nIndex <= VS2017_VERSION; nIndex++) {
		m_hSolutionBitmaps[nIndex] = nullptr;
	}

	m_hSolutionBitmaps[VS2005_VERSION] = ::LoadImage(g_hInst, MAKEINTRESOURCE(IDB_VS2005), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
	m_hSolutionBitmaps[VS2008_VERSION] = ::LoadImage(g_hInst, MAKEINTRESOURCE(IDB_VS2008), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
	m_hSolutionBitmaps[VS2010_VERSION] = ::LoadImage(g_hInst, MAKEINTRESOURCE(IDB_VS2010), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
	m_hSolutionBitmaps[VS2012_VERSION] = ::LoadImage(g_hInst, MAKEINTRESOURCE(IDB_VS2012), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
	m_hSolutionBitmaps[VS2013_VERSION] = ::LoadImage(g_hInst, MAKEINTRESOURCE(IDB_VS2013), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
	m_hSolutionBitmaps[VS2015_VERSION] = ::LoadImage(g_hInst, MAKEINTRESOURCE(IDB_VS2015), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);
	m_hSolutionBitmaps[VS2017_VERSION] = ::LoadImage(g_hInst, MAKEINTRESOURCE(IDB_VS2017), IMAGE_BITMAP, 16, 16, LR_LOADTRANSPARENT);


}

CVsShellContextMenuHandler::~CVsShellContextMenuHandler()
{
	if (m_hSettingsMenuBmp)
	{
		DeleteObject(m_hSettingsMenuBmp);
		m_hSettingsMenuBmp = NULL;
	}

	for (UINT nIndex = VS2005_VERSION; nIndex <= VS2017_VERSION; nIndex++) {
		DeleteObject(m_hSolutionBitmaps[nIndex]);
		m_hSolutionBitmaps[nIndex] = nullptr;
	}

	if (nullptr != m_hSubMenu) {
		DestroyMenu(m_hSubMenu);
		m_hSubMenu = nullptr;
	}

	InterlockedDecrement(&g_cDllRef);
}

void CVsShellContextMenuHandler::OnVerb(HWND hWnd)
{
	CDebugLogger::WriteInfo(__FUNCTIONW__);
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP CVsShellContextMenuHandler::QueryInterface(REFIID riid, void **ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(CVsShellContextMenuHandler, IContextMenu),
		QITABENT(CVsShellContextMenuHandler, IShellExtInit),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) CVsShellContextMenuHandler::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) CVsShellContextMenuHandler::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (0 == cRef)
	{
		delete this;
	}

	return cRef;
}

#pragma endregion


#pragma region IShellExtInit

// Initialize the context menu handler.
IFACEMETHODIMP CVsShellContextMenuHandler::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID)
{
	if (NULL == pDataObj)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = E_FAIL;

	FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stm;

	// The pDataObj pointer contains the objects being acted upon. In this 
	// example, we get an HDROP handle for enumerating the selected files and 
	// folders.
	if (SUCCEEDED(pDataObj->GetData(&fe, &stm)))
	{
		// Get an HDROP handle.
		HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));
		if (hDrop != NULL)
		{
			// Determine how many files are involved in this operation. This 
			// code sample displays the custom context menu item when only 
			// one file is selected. 
			UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			if (nFiles == 1)
			{
				// Get the path of the file.
				if (0 != DragQueryFile(hDrop, 0, m_szSelectedFile, ARRAYSIZE(m_szSelectedFile)))
				{
					//CDebugLogger::WriteInfo(_T("m_szSelectedFile=%s"), m_szSelectedFile);
					m_File.Close();
					if (m_File.Open(m_szSelectedFile) == EVsSolutionFileError_NoErr) {
						hr = S_OK;
					}
					else {
						CDebugLogger::WriteError(_T("%s[%d]"), __FUNCTIONW__, __LINE__);
						hr = S_FALSE;
					}
				}
			}

			GlobalUnlock(stm.hGlobal);
		}

		ReleaseStgMedium(&stm);
	}

	// If any value other than S_OK is returned from the method, the context 
	// menu item is not displayed.
	return hr;
}

#pragma endregion


#pragma region IContextMenu

//
//   FUNCTION: CVsShellContextMenuHandler::QueryContextMenu
//
//   PURPOSE: The Shell calls IContextMenu::QueryContextMenu to allow the 
//            context menu handler to add its menu items to the menu. It 
//            passes in the HMENU handle in the hmenu parameter. The 
//            indexMenu parameter is set to the index to be used for the 
//            first menu item that is to be added.
//
IFACEMETHODIMP CVsShellContextMenuHandler::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	// If uFlags include CMF_DEFAULTONLY then we should not do anything.
	if (CMF_DEFAULTONLY & uFlags)
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	// Use either InsertMenu or InsertMenuItem to add menu items.
	// Learn how to add sub-menu from:
	// http://www.codeproject.com/KB/shell/ctxextsubmenu.aspx

	UINT nMainMenuCount = 0;
	// Add a separator.
	MENUITEMINFO sep = { sizeof(sep) };
	sep.fMask = MIIM_TYPE;
	sep.fType = MFT_SEPARATOR;
	if (!InsertMenuItem(hMenu, indexMenu + nMainMenuCount++, TRUE, &sep))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}


	if (m_File.Valid()) {
		CVsSolution* pSolution = m_File.Solution();
		if (nullptr != pSolution) {
			UINT nSubMenuItem = 0;
			if (nullptr != m_hSubMenu) {
				DestroyMenu(m_hSubMenu);
				m_hSubMenu = nullptr;
			}

			m_hSubMenu = CreatePopupMenu();

			MENUITEMINFO mainItem = { sizeof(mainItem) };
			mainItem.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_SUBMENU;
			mainItem.wID = idCmdFirst + nMainMenuCount;
			mainItem.fType = MFT_STRING;
			mainItem.dwTypeData = (PWSTR)m_File.Solution()->Name();
			mainItem.fState = MFS_ENABLED;
			mainItem.hbmpItem = static_cast<HBITMAP>(m_hSolutionBitmaps[pSolution->Version().Major]);
			mainItem.hSubMenu = m_hSubMenu;
			
			if (!InsertMenuItem(hMenu, indexMenu + nMainMenuCount++, TRUE, &mainItem))
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}

			CVsProject* pProj = pSolution->firstProject();
			if (pProj != nullptr) {
				while (nullptr != pProj) {
					MENUITEMINFO miim = { sizeof(miim) };
					miim.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE;
					miim.fType = MFT_STRING;
					miim.dwTypeData = (PWSTR)pProj->Name();
					miim.fState = MFS_ENABLED;

					if (!InsertMenuItem(m_hSubMenu, nSubMenuItem++, TRUE, &miim))
					{
						return HRESULT_FROM_WIN32(GetLastError());
					}

					pProj = pSolution->nextProject();
				}
			}
		}
	}

	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
	mii.wID = idCmdFirst + nMainMenuCount;
	mii.fType = MFT_STRING;
	mii.dwTypeData = (PWSTR)L"VsShellContextMenu Settings...";
	mii.fState = MFS_ENABLED;
	mii.hbmpItem = static_cast<HBITMAP>(m_hSettingsMenuBmp);
	if (!InsertMenuItem(hMenu, indexMenu + nMainMenuCount++, TRUE, &mii))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// Add a separator.
	if (!InsertMenuItem(hMenu, indexMenu + nMainMenuCount++, TRUE, &sep))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_DISPLAY + 1));
}


//
//   FUNCTION: CVsShellContextMenuHandler::InvokeCommand
//
//   PURPOSE: This method is called when a user clicks a menu item to tell 
//            the handler to run the associated command. The lpcmi parameter 
//            points to a structure that contains the needed information.
//
IFACEMETHODIMP CVsShellContextMenuHandler::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
	BOOL fUnicode = FALSE;

	// Determine which structure is being passed in, CMINVOKECOMMANDINFO or 
	// CMINVOKECOMMANDINFOEX based on the cbSize member of lpcmi. Although 
	// the lpcmi parameter is declared in Shlobj.h as a CMINVOKECOMMANDINFO 
	// structure, in practice it often points to a CMINVOKECOMMANDINFOEX 
	// structure. This struct is an extended version of CMINVOKECOMMANDINFO 
	// and has additional members that allow Unicode strings to be passed.
	if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
	{
		if (pici->fMask & CMIC_MASK_UNICODE)
		{
			fUnicode = TRUE;
		}
	}

	// Determines whether the command is identified by its offset or verb.
	// There are two ways to identify commands:
	// 
	//   1) The command's verb string 
	//   2) The command's identifier offset
	// 
	// If the high-order word of lpcmi->lpVerb (for the ANSI case) or 
	// lpcmi->lpVerbW (for the Unicode case) is nonzero, lpVerb or lpVerbW 
	// holds a verb string. If the high-order word is zero, the command 
	// offset is in the low-order word of lpcmi->lpVerb.

	// For the ANSI case, if the high-order word is not zero, the command's 
	// verb string is in lpcmi->lpVerb. 
	if (!fUnicode && HIWORD(pici->lpVerb))
	{
		// Is the verb supported by this context menu extension?
		if (StrCmpIA(pici->lpVerb, m_pszVerb) == 0)
		{
			OnVerb(pici->hwnd);
		}
		else
		{
			// If the verb is not recognized by the context menu handler, it 
			// must return E_FAIL to allow it to be passed on to the other 
			// context menu handlers that might implement that verb.
			return E_FAIL;
		}
	}

	// For the Unicode case, if the high-order word is not zero, the 
	// command's verb string is in lpcmi->lpVerbW. 
	else if (fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
	{
		// Is the verb supported by this context menu extension?
		if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, m_pwszVerb) == 0)
		{
			OnVerb(pici->hwnd);
		}
		else
		{
			// If the verb is not recognized by the context menu handler, it 
			// must return E_FAIL to allow it to be passed on to the other 
			// context menu handlers that might implement that verb.
			return E_FAIL;
		}
	}

	// If the command cannot be identified through the verb string, then 
	// check the identifier offset.
	else
	{
		// Is the command identifier offset supported by this context menu 
		// extension?
		if (LOWORD(pici->lpVerb) == IDM_DISPLAY)
		{
			OnVerb(pici->hwnd);
		}
		else
		{
			// If the verb is not recognized by the context menu handler, it 
			// must return E_FAIL to allow it to be passed on to the other 
			// context menu handlers that might implement that verb.
			return E_FAIL;
		}
	}

	return S_OK;
}


//
//   FUNCTION: CFileContextMenuExt::GetCommandString
//
//   PURPOSE: If a user highlights one of the items added by a context menu 
//            handler, the handler's IContextMenu::GetCommandString method is 
//            called to request a Help text string that will be displayed on 
//            the Windows Explorer status bar. This method can also be called 
//            to request the verb string that is assigned to a command. 
//            Either ANSI or Unicode verb strings can be requested. This 
//            example only implements support for the Unicode values of 
//            uFlags, because only those have been used in Windows Explorer 
//            since Windows 2000.
//
IFACEMETHODIMP CVsShellContextMenuHandler::GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
	HRESULT hr = E_INVALIDARG;

	if (idCommand == IDM_DISPLAY)
	{
		switch (uFlags)
		{
		case GCS_HELPTEXTW:
			// Only useful for pre-Vista versions of Windows that have a 
			// Status bar.
			hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, m_pwszVerbHelpText);
			break;

		case GCS_VERBW:
			// GCS_VERBW is an optional feature that enables a caller to 
			// discover the canonical name for the verb passed in through 
			// idCommand.
			hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax,
				m_pwszVerbCanonicalName);
			break;

		default:
			hr = S_OK;
		}
	}

	// If the command (idCommand) is not supported by this context menu 
	// extension handler, return E_INVALIDARG.

	return hr;
}