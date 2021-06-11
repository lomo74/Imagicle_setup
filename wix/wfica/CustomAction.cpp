/*
Imagicle print2fax
Copyright (C) 2021 Lorenzo Monti

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "stdafx.h"
#include <ShlObj.h>
#include <string>
#include "Utils.h"
//---------------------------------------------------------------------------

#define ISSLASH(a) ((a) == L'\\')
#define COUNTOF(x) (sizeof(x)/sizeof((x)[0]))
//---------------------------------------------------------------------------

static BOOL DirectoryExists(LPCWSTR szDirName)
{
	DWORD dwAttr = GetFileAttributesW(szDirName);
	return (dwAttr != INVALID_FILE_ATTRIBUTES) &&
		((dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}
//-------------------------------------------------------------------------------------

void GetFileParent(LPCWSTR szFile, LPWSTR szParent, size_t count)
{
	size_t i, len;
	i = len = wcslen(szFile) - 1;
	/*go back until we encounter a colon or backslash(es)*/
	BOOL bSlashSaw = FALSE;

	while (i != 0)
	{
		if (szFile[i] == L':')
			break;
		else if (ISSLASH(szFile[i]))
		{
			if (i == 1 && ISSLASH(szFile[0]))
			{
				i = len;
				break;
			}
			else
				bSlashSaw = TRUE; //begin to eat backslashes
		}
		else if (bSlashSaw)
			break; //last backslash eaten
		i--;
	}

	if (i < count - 1)
	{
		szParent[i + 1] = L'\0';
		wmemcpy_s(szParent, count, szFile, i + 1);
	}
	else
		szParent[0] = L'\0'; //should never occur...
}
//-------------------------------------------------------------------------------------

static DWORD RecursiveCreateFolder(LPCWSTR szPath)
{
	WCHAR szPathBuf[MAX_PATH + 1];
	WCHAR szParent[MAX_PATH + 1];
	LPCWSTR pPath = szPath;
	size_t len;

	/*strip off leading backslashes*/
	len = wcslen(szPath);
	if (len > 0 && ISSLASH(szPath[len - 1]))
	{
		/*make a copy of szPath only if needed*/
		wcscpy_s(szPathBuf, COUNTOF(szPathBuf), szPath);
		pPath = szPathBuf;
		while (len > 0 && ISSLASH(szPathBuf[len - 1]))
		{
			szPathBuf[len - 1] = L'\0';
			len--;
		}
	}
	/*only drive letter left or the directory already exists*/
	if (len < 3 || DirectoryExists(pPath))
		return ERROR_SUCCESS;
	else
	{
		GetFileParent(pPath, szParent, COUNTOF(szParent));
		if (wcscmp(pPath, szParent) == 0)
			return ERROR_SUCCESS;
		/*our parent must exist before we can get created*/
		DWORD dwRet = RecursiveCreateFolder(szParent);
		if (dwRet != ERROR_SUCCESS)
			return dwRet;
		if (!CreateDirectoryW(pPath, NULL))
			return GetLastError();
		return ERROR_SUCCESS;
	}
}
//-------------------------------------------------------------------------------------

UINT __stdcall ReadIni(
	MSIHANDLE hInstall
)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;
	WCHAR buf[MAX_PATH + 1] = { 0 };
	std::wstring path;
	const DWORD DATASIZE = 2048;
	WCHAR data[DATASIZE];
	DWORD nchars;
	LPCWSTR szSection = L"PARAMETERS";
	//std::wstring plain;

	hr = WcaInitialize(hInstall, "ReadIni");
	ExitOnFailure(hr, "Failed to initialize");

	WcaLog(LOGMSG_STANDARD, "Initialized.");

	hr = SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf);
	ExitOnFailure(hr, "Failed to get common appdata path");

	path = buf;
	path += L"\\Imagicle print2fax\\wfigui.ini";

	nchars = GetPrivateProfileStringW(szSection, L"Server", L"", data, DATASIZE, path.c_str());
	if (nchars >= DATASIZE - 1)
	{
		hr = E_INSUFFICIENT_BUFFER;
		ExitOnFailure(hr, "Failed to read ini file");
	}
	hr = WcaSetProperty(L"INI_SERVER", data);
	ExitOnFailure(hr, "Failed to set property");

	if (GetPrivateProfileIntW(szSection, L"UseSSL", 1, path.c_str()))
	{
		hr = WcaSetIntProperty(L"INI_USESSL", 1);
		ExitOnFailure(hr, "Failed to set property");
	}

	if (GetPrivateProfileIntW(szSection, L"SkipCertificateCheck", 0, path.c_str()))
	{
		hr = WcaSetIntProperty(L"INI_SKIPCERTIFICATECHECK", 1);
		ExitOnFailure(hr, "Failed to set property");
	}

	/*
	nchars = GetPrivateProfileStringW(szSection, L"Username", L"", data, DATASIZE, path.c_str());
	if (nchars >= DATASIZE - 1)
	{
		hr = E_INSUFFICIENT_BUFFER;
		ExitOnFailure(hr, "Failed to read ini file");
	}
	hr = WcaSetProperty(L"INI_USERNAME", data);
	ExitOnFailure(hr, "Failed to set property");

	nchars = GetPrivateProfileStringW(szSection, L"Password", L"", data, DATASIZE, path.c_str());
	if (nchars >= DATASIZE - 1)
	{
		hr = E_INSUFFICIENT_BUFFER;
		ExitOnFailure(hr, "Failed to read ini file");
	}
	plain = DecryptString(data);
	hr = WcaSetProperty(L"INI_PASSWORD", plain.c_str());
	ExitOnFailure(hr, "Failed to set property");
	*/

LExit:
	er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
	return WcaFinalize(er);
}
//---------------------------------------------------------------------------

UINT __stdcall WriteIni(
	MSIHANDLE hInstall
	)
{
	HRESULT hr = S_OK;
	UINT er = ERROR_SUCCESS;
	TCHAR buf[MAX_PATH + 1] = { 0 };
	std::wstring path;
	LPWSTR pszData = NULL;
	LPCWSTR szSection = L"PARAMETERS";
	//std::wstring cipher;

	hr = WcaInitialize(hInstall, "WriteIni");
	ExitOnFailure(hr, "Failed to initialize");

	WcaLog(LOGMSG_STANDARD, "Initialized.");

	hr = SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf);
	ExitOnFailure(hr, "Failed to get common appdata path");

	path = buf;
	path += L"\\Imagicle print2fax";

	hr = HRESULT_FROM_WIN32(RecursiveCreateFolder(path.c_str()));
	ExitOnFailure(hr, "Failed to create ini file path");

	path += L"\\wfigui.ini";

	hr = WcaGetProperty(L"INI_SERVER", &pszData);
	ExitOnFailure(hr, "Failed to get property value");
	WritePrivateProfileStringW(szSection, L"Server", pszData, path.c_str());

	if (WcaIsUnicodePropertySet(L"INI_USESSL"))
		WritePrivateProfileStringW(szSection, L"UseSSL", L"1", path.c_str());
	else
		WritePrivateProfileStringW(szSection, L"UseSSL", L"0", path.c_str());

	if (WcaIsUnicodePropertySet(L"INI_SKIPCERTIFICATECHECK"))
		WritePrivateProfileStringW(szSection, L"SkipCertificateCheck", L"1", path.c_str());
	else
		WritePrivateProfileStringW(szSection, L"SkipCertificateCheck", L"0", path.c_str());

	/*
	hr = WcaGetProperty(L"INI_USERNAME", &pszData);
	ExitOnFailure(hr, "Failed to get property value");
	WritePrivateProfileStringW(szSection, L"Username", pszData, path.c_str());

	hr = WcaGetProperty(L"INI_PASSWORD", &pszData);
	ExitOnFailure(hr, "Failed to get property value");
	cipher = EncryptString(pszData);
	WritePrivateProfileStringW(szSection, L"Password", cipher.c_str(), path.c_str());
	*/

LExit:
	er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
	return WcaFinalize(er);
}
//---------------------------------------------------------------------------

// DllMain - Initialize and cleanup WiX custom action utils.
extern "C" BOOL WINAPI DllMain(
	__in HINSTANCE hInst,
	__in ULONG ulReason,
	__in LPVOID
	)
{
	switch(ulReason)
	{
	case DLL_PROCESS_ATTACH:
		WcaGlobalInitialize(hInst);
		break;

	case DLL_PROCESS_DETACH:
		WcaGlobalFinalize();
		break;
	}

	return TRUE;
}
//---------------------------------------------------------------------------
