#pragma once
#include "Forwards.h"

namespace IO
{
	inline  bool is_exist_dir(String& dir)
	{
		const auto func_GetFileAttributesA = WinApiImport<f_GetFileAttributesA>::get_func("GetFileAttributesA", "Kernel32.dll");
		DWORD dwAttr = func_GetFileAttributesA(dir.c_str());
		if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) { return true; }
		return false;
	}

	
	inline  bool is_file_exists(const std::string& file)
	{
		const auto func_FindFirstFile = WinApiImport<f_FindFirstFile>::get_func("FindFirstFile", "kernel32.dll");
		const auto func_FindClose = WinApiImport<f_FindClose>::get_func("FindClose", "kernel32.dll");

		WIN32_FIND_DATA FindFileData;
		HANDLE handle = func_FindFirstFile(file.c_str(), &FindFileData);
		int found = handle != INVALID_HANDLE_VALUE;
		if (found)
		{
			//FindClose(&handle); this will crash
			func_FindClose(handle);
		}
		return found;
	}


	inline bool read_file(const String& path_to_file, String& out_data)
	{
		if(!is_file_exists(path_to_file)) return false;

		const auto func_GetFileSize = WinApiImport<f_GetFileSize>::get_func("GetFileSize", "kernel32.dll");
		const auto func_CreateFileA = WinApiImport<f_CreateFileA>::get_func("CreateFileA", "kernel32.dll");
		const auto func_CloseHandle = WinApiImport<f_CloseHandle>::get_func("CloseHandle", "kernel32.dll");
		const auto func_ReadFile = WinApiImport<f_ReadFile>::get_func("ReadFile", "kernel32.dll");
		
		auto hFile = func_CreateFileA(path_to_file.c_str(), GENERIC_READ, 0, 0, OPEN_ALWAYS, 0, 0);
		
		if (hFile == INVALID_HANDLE_VALUE)		
			return false;

		const auto fileSize = func_GetFileSize(hFile, NULL);

		if (fileSize == 0)
			return false;
		
		auto tempBuff = new char[fileSize];
		
		DWORD numToRead;
		if (!func_ReadFile(hFile, tempBuff, fileSize, &numToRead, NULL))
			return false;


		//tempBuff[numToRead] = L'\0';
		out_data = tempBuff;
		out_data.resize(numToRead);		
		delete[] tempBuff;
		tempBuff = nullptr;
		
		if (!func_CloseHandle(hFile))
			return false;
		
		return true;
	}

	inline void remove_directory(LPCTSTR dir) // Fully qualified name of the directory being deleted, without trailing backslash
	{
		try
		{
			const auto func_SHFileOperation = WinApiImport<f_SHFileOperation>::get_func("SHFileOperation", "shell32.dll");

			SHFILEOPSTRUCT file_op = {
				NULL,
				FO_DELETE,
				dir,
				"",
				FOF_NOCONFIRMATION |
				FOF_NOERRORUI |
				FOF_SILENT,
				false,
				0,
				"" };
			auto result = func_SHFileOperation(&file_op);
			int z = 0;
		}
		catch (...)
		{
		}
		
	}

	
	inline void get_subdirs(List<String>& output, const String& path)
	{
		const auto func_GetFullPathName = WinApiImport<f_GetFullPathName>::get_func("GetFullPathName", "kernel32.dll");
		const auto func_FindFirstFile = WinApiImport<f_FindFirstFile>::get_func("FindFirstFile", "kernel32.dll");
		const auto func_FindNextFile = WinApiImport<f_FindNextFile>::get_func("FindNextFile", "kernel32.dll");


		WIN32_FIND_DATA findfiledata;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		char fullpath[MAX_PATH];
		func_GetFullPathName(path.c_str(), MAX_PATH, fullpath, 0);
		std::string fp(fullpath);

		hFind = func_FindFirstFile((LPCSTR)(fp + "\\*").c_str(), &findfiledata);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if ((findfiledata.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY
					&& (findfiledata.cFileName[0] != '.'))
				{
					output.push_back(findfiledata.cFileName);
				}
			} while (func_FindNextFile(hFind, &findfiledata) != 0);
		}
	}


	inline bool create_directory_recursively(LPCTSTR path)
	{
		const auto func_SHCreateDirectoryEx = WinApiImport<f_SHCreateDirectoryEx>::get_func("SHCreateDirectoryEx", "shell32.dll");		
		return func_SHCreateDirectoryEx(NULL, path, NULL) == ERROR_SUCCESS;
	}


	inline bool copy_file(const String& from, const String& to)
	{
		const auto func_CopyFileA = WinApiImport<f_CopyFileA>::get_func("CopyFileA", "kernel32.dll");
		return func_CopyFileA(from.c_str(), to.c_str(), false);
	}


	inline String get_app_folder(int CSIDL_FLAG = CSIDL_APPDATA)
	{
		const auto get_user_path = WinApiImport<f_SHGetFolderPathA>::get_func("SHGetFolderPathA", "shell32.dll");

		if (!get_user_path) return {};
		char m_path_local_data[MAX_PATH];

		if (get_user_path(NULL, CSIDL_FLAG, NULL, 0, m_path_local_data) != S_OK) return "";
		
		return m_path_local_data;
	}

	inline String get_temp_folder()
	{
		const auto get_temp_folder = WinApiImport<f_GetTempPathA>::get_func("GetTempPathA", "kernel32.dll");

		char wcharPath[MAX_PATH];
		if (get_temp_folder(MAX_PATH, wcharPath))
			return wcharPath;
		
		return "";
	}
	
}
