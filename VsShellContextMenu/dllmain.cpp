// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "VsShellContextMenuFactory.h"
#include "ShellUtility.h"
#include "DebugLogger.h"
#include <Guiddef.h>

const CLSID CLSID_VsShellContextMenu = { 0x5d8f544b, 0x1593, 0x4c16, { 0xae, 0xc1, 0xb1, 0xa6, 0x80, 0x24, 0xa3, 0xce } };


HINSTANCE   g_hInst = NULL;
long        g_cDllRef = 0;

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		g_hInst = hModule;
		DisableThreadLibraryCalls(hModule);
		CDebugLogger::SetLogPath(_T("D:\\VsShellContextMenu.log"));
		CDebugLogger::WriteInfo(__FUNCTIONW__);
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		CDebugLogger::Close();
        break;
    }
    return TRUE;
}


//
//   FUNCTION: DllGetClassObject
//
//   PURPOSE: Create the class factory and query to the specific interface.
//
//   PARAMETERS:
//   * rclsid - The CLSID that will associate the correct data and code.
//   * riid - A reference to the identifier of the interface that the caller 
//     is to use to communicate with the class object.
//   * ppv - The address of a pointer variable that receives the interface 
//     pointer requested in riid. Upon successful return, *ppv contains the 
//     requested interface pointer. If an error occurs, the interface pointer 
//     is NULL. 
//
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if (IsEqualCLSID(CLSID_VsShellContextMenu, rclsid))
	{
		hr = E_OUTOFMEMORY;

		CVsShellContextMenuFactory *pClassFactory = new CVsShellContextMenuFactory();
		if (pClassFactory)
		{
			hr = pClassFactory->QueryInterface(riid, ppv);
			pClassFactory->Release();
		}
	}

	return hr;
}


//
//   FUNCTION: DllCanUnloadNow
//
//   PURPOSE: Check if we can unload the component from the memory.
//
//   NOTE: The component can be unloaded from the memory when its reference 
//   count is zero (i.e. nobody is still using the component).
// 
STDAPI DllCanUnloadNow(void)
{
	return g_cDllRef > 0 ? S_FALSE : S_OK;
}


//
//   FUNCTION: DllRegisterServer
//
//   PURPOSE: Register the COM server and the context menu handler.
// 
STDAPI DllRegisterServer(void)
{
	HRESULT hr;

	wchar_t szModule[MAX_PATH];
	if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	// Register the component.
	hr = CShellUtility::RegisterInprocServer(szModule, CLSID_VsShellContextMenu, L"VsShellContextMenu.Handler.Class", L"Apartment");
	if (SUCCEEDED(hr))
	{
		// Register the context menu handler. The context menu handler is 
		// associated with the .sln file class.
		hr = CShellUtility::RegisterShellExtContextMenuHandler(SUPPORTED_FILE_EXTENSION, CLSID_VsShellContextMenu, L"VsShellContextMenu.Handler");
	}

	return hr;
}


//
//   FUNCTION: DllUnregisterServer
//
//   PURPOSE: Unregister the COM server and the context menu handler.
// 
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = S_OK;

	wchar_t szModule[MAX_PATH];
	if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	// Unregister the component.
	hr = CShellUtility::UnregisterInprocServer(CLSID_VsShellContextMenu);
	if (SUCCEEDED(hr))
	{
		// Unregister the context menu handler.
		hr = CShellUtility::UnregisterShellExtContextMenuHandler(SUPPORTED_FILE_EXTENSION, CLSID_VsShellContextMenu);
	}

	return hr;
}