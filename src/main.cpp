#include <iostream>
#include "ChromeParser.h"
#include "firefox_parser.h"
#include "FactoryCollector.h"


void print_results(const List<AccountData>& data_accounts)
{
	for (const auto& data : data_accounts)
	{
		std::cout << "Url: " << data.Url << std::endl << "Username: " << data.Username << std::endl << "Password: " << data.Password << std::endl;
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
	}
}

void clear(ICollector* collector)
{
	if (!collector) return;
	delete collector;
	collector = nullptr;
}

int main()
{
	
	auto parser = collector::create_collector(collector::BrowserType::Mozilla);	
	print_results(parser->collect_data());
	clear(parser);
	
	parser = collector::create_collector(collector::BrowserType::Chrome);
	print_results(parser->collect_data());
	clear(parser);
	
	
	system("pause");
	return 0;
}
