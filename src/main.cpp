#include <iostream>
#include "ChromeParser.h"
#include "FireFoxParser.h"
#include "FactoryCollector.h"


void print_result(const List<AccountData>& data)
{
	for (const auto& data : data)
	{
		std::cout << "Url: " << data.Url << std::endl << "Username: " << data.Username << std::endl << "Password: " << data.Password << std::endl;
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
	}
}

void print_result(const List<CookieData>& data)
{
	for (const auto& data : data)
	{
		std::cout << "HostKey: " << data.HostKey << std::endl << "Name: " << data.Name << std::endl << "Value: " << data.Value << std::endl << "Path: " << data.Path << std::endl << "ExpireUTC: " << data.ExpireUTC << std::endl;
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
	}
}

template<class T>
void clear(ICollector<T>* collector)
{
	if (!collector) return;
	delete collector;
	collector = nullptr;
}

int main()
{	
	auto parser = collector::create_password_collector(collector::BrowserType::Mozilla);	
	print_result(parser->collect_data());
	clear(parser);
	
	parser = collector::create_password_collector(collector::BrowserType::Chromium);
	print_result(parser->collect_data());
	clear(parser);

	
	auto parser_cookie = collector::create_cookies_collector(collector::BrowserType::Mozilla);
	print_result(parser_cookie->collect_data());
	clear(parser_cookie);

	parser_cookie = collector::create_cookies_collector(collector::BrowserType::Chromium);
	print_result(parser_cookie->collect_data());
	clear(parser_cookie);
	
	
	system("pause");
	return 0;
}
