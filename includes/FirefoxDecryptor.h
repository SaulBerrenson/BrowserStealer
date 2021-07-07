#pragma once
#include "Forwards.h"

enum class SECItemType {
    siBuffer = 0,
    siClearDataBuffer = 1,
    siCipherDataBuffer,
    siDERCertBuffer,
    siEncodedCertBuffer,
    siDERNameBuffer,
    siEncodedNameBuffer,
    siAsciiNameString,
    siAsciiString,
    siDEROID,
    siUnsignedInteger,
    siUTCTime,
    siGeneralizedTime
};

struct SECItem {
    SECItemType type;
    unsigned char* data;
    size_t len;
};

using Pk11SdrDecrypt = int(SECItem*, SECItem*, void*);
using NssInit = long(char* sDirectory);
using NssShutdown = long();


class FirefoxDecryptor
{
public:
    ~FirefoxDecryptor();
	
	bool init(const String& sPath);
    bool set_profile_dir(const String& profile_dir);
    bool decrypt_data(const String& encrypted_data, String& decrypted_data);
	
    NssInit* m_NssInit = nullptr;
    Pk11SdrDecrypt* m_ipNssPk11SdrDecrypt = nullptr;
    NssShutdown* m_NssShutdown = nullptr;

   HMODULE m_hNss3 = nullptr;
   HMODULE m_hMozGlue = nullptr;
	
};
