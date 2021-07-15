#pragma once
#include "Forwards.h"

class IDecryptor
{
public:
	virtual ~IDecryptor() = default;

	virtual bool init(const String& sPath) = 0;
	virtual bool decrypt_data(const String& encrypted_data, String& decrypted_data) = 0;	
};


