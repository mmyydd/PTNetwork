#include <iostream>
#include <string>
#include <json/json.h>



int main()
{
	Json::Value root;


	root["firstKey"] = "1111";

	root["secondKey"] = 123456;


	std::cout << root.toStyledString() << std::endl;

	return 0;
}

