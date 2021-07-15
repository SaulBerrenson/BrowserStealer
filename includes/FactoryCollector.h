#pragma once
#include "ICollector.h"
#include "ChromeParser.h"
#include "FireFoxParser.h"

namespace collector
{
	enum class BrowserType
	{
		None,
		Chromium = 32,
		Mozilla = 64,
	};

	inline ICollector* create_collector(BrowserType type_collector)
	{
		switch (type_collector)
		{
			case BrowserType::None: return nullptr;
			case BrowserType::Chromium: return new ChromiumParser();
			case BrowserType::Mozilla: return new FireFoxParser();
			default: return nullptr;;
		}
	}
	
}
