#include <iostream>
#include "ChromeParser.h"
#include "firefox_parser.h"
#include "FactoryCollector.h"

int main()
{
	



	

	auto firefox_parser = collector::create_collector(collector::BrowserType::Mozilla);

	auto accounts = firefox_parser->collect_data();

	for (const auto& data : accounts)
	{
		std::cout << "Url: " << data.Url << std::endl << "Username: " << data.Username << std::endl << "Password: " << data.Password << std::endl;
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
	}

	if(firefox_parser)
	{
		delete firefox_parser;
		firefox_parser = nullptr;
	}
	
	
	auto chrome_parser = collector::create_collector(collector::BrowserType::Chrome);

	for (const auto& data : chrome_parser->collect_data())
	{
		std::cout << "Url: " <<data.Url << std::endl << "Username: " << data.Username << std::endl << "Password: " << data.Password << std::endl;
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
	}

	if (chrome_parser)
	{
		delete chrome_parser;
		chrome_parser = nullptr;
	}
	
	system("pause");
	return 0;
}
