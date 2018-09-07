#pragma once
class CShellUtility
{
private:
	static HRESULT SetHKCRRegistryKeyAndValue(PCWSTR pszSubKey, PCWSTR pszValueName, PCWSTR pszData);
	static HRESULT GetHKCRRegistryKeyAndValue(PCWSTR pszSubKey, PCWSTR pszValueName, PWSTR pszData, DWORD cbData);
public:
	static HRESULT RegisterInprocServer(PCWSTR pszModule, const CLSID& clsid, PCWSTR pszFriendlyName, PCWSTR pszThreadModel);
	static HRESULT UnregisterInprocServer(const CLSID& clsid);
	static HRESULT RegisterShellExtContextMenuHandler(PCWSTR pszFileType, const CLSID& clsid, PCWSTR pszFriendlyName);
	static HRESULT UnregisterShellExtContextMenuHandler(PCWSTR pszFileType, const CLSID& clsid);
};

