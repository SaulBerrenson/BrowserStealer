#pragma once
#include <string>
#include "Forwards.h"


namespace regedit_helper
{
    static List<String> get_subkeys(HKEY RootKey, char* subKey, unsigned int tabs = 0)
    {

        const auto func_RegOpenKeyEx = WinApiImport<f_RegOpenKeyEx>::get("RegOpenKeyEx", "advapi32.dll");
        const auto func_RegQueryInfoKey = WinApiImport<f_RegQueryInfoKey>::get("RegQueryInfoKey", "advapi32.dll");
        const auto func_RegEnumKeyEx = WinApiImport<f_RegEnumKeyEx>::get("RegEnumKeyEx", "advapi32.dll");
        const auto func_RegCloseKey = WinApiImport<f_RegCloseKey>::get("RegCloseKey", "advapi32.dll");

        List<String> list_subkeys;

        HKEY hKey;
        DWORD cSubKeys;        //Used to store the number of Subkeys
        DWORD maxSubkeyLen;    //Longest Subkey name length
        DWORD cValues;        //Used to store the number of Subkeys
        DWORD maxValueLen;    //Longest Subkey name length
        DWORD retCode;        //Return values of calls

        func_RegOpenKeyEx(RootKey, subKey, 0, KEY_ALL_ACCESS, &hKey);

        func_RegQueryInfoKey(hKey,
            NULL,            // buffer for class name
            NULL,            // size of class string
            NULL,            // reserved
            &cSubKeys,        // number of subkeys
            &maxSubkeyLen,    // longest subkey length
            NULL,            // longest class string 
            &cValues,        // number of values for this key 
            &maxValueLen,    // longest value name 
            NULL,            // longest value data 
            NULL,            // security descriptor 
            NULL);            // last write time

        if (cSubKeys > 0)
        {
            char currentSubkey[MAX_PATH];

            for (int i = 0; i < cSubKeys; i++) {
                DWORD currentSubLen = MAX_PATH;

                retCode = func_RegEnumKeyEx(hKey,    // Handle to an open/predefined key
                    i,                // Index of the subkey to retrieve.
                    currentSubkey,            // buffer to receives the name of the subkey
                    &currentSubLen,            // size of that buffer
                    NULL,                // Reserved
                    NULL,                // buffer for class string 
                    NULL,                // size of that buffer
                    NULL);                // last write time

                if (retCode == ERROR_SUCCESS)
                {
					String path(subKey);
                    path.append("\\").append(currentSubkey).append("\\Capabilities");
                    list_subkeys.emplace_back(path);
                }
            }
        }

        func_RegCloseKey(hKey);

        return list_subkeys;
    }

    static String ReadRegValue(HKEY root, String key, String name)
    {
        const auto func_RegOpenKeyEx = WinApiImport<f_RegOpenKeyEx>::get("RegOpenKeyEx", "advapi32.dll");
        const auto func_f_RegQueryValueEx = WinApiImport<f_RegQueryValueEx>::get("RegQueryValueEx", "advapi32.dll");
        const auto func_RegCloseKey = WinApiImport<f_RegCloseKey>::get("RegCloseKey", "advapi32.dll");    	
    	
        HKEY hKey;
        if (func_RegOpenKeyEx(root, key.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
            return "";

        DWORD type;
        DWORD cbData;
        if (func_f_RegQueryValueEx(hKey, name.c_str(), NULL, &type, NULL, &cbData) != ERROR_SUCCESS)
        {
            func_RegCloseKey(hKey);
            return "";
        }        

        String value(cbData / sizeof(char), L'\0');
        if (func_f_RegQueryValueEx(hKey, name.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&value[0]), &cbData) != ERROR_SUCCESS)
        {
            func_RegCloseKey(hKey);
            return "";
        }

        func_RegCloseKey(hKey);

        auto firstNull = value.find_last_of("\\");
        if (firstNull != String::npos)
            value.resize(firstNull);

        return value;
    }

    static List<String> subkeys_contains(List<String>& subkeys, String condition)
    {
        List<String> list_subkeys;

    	for (const auto& key : subkeys)
    	{
            if (key.find(condition) != std::string::npos) {
                list_subkeys.emplace_back(key);
            }
    	}

        return list_subkeys;
    }


	


    static List<String> get_path(List<String>& subkeys)
    {
        List<String> list_subkeys;

        for (const auto& key : subkeys)
        {
            int z = 0;
        }

        return list_subkeys;
    }

	
}
