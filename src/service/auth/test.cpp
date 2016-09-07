#include "base64.hpp"

#include <iostream>


int main()
{
	//std::string decode_data = "1|cuchzhcuizhxciuhuihauc";
	std::string decode_data = "1|";
	int index = decode_data.find("|");

	std::cout << index << std::endl;


	if(index == std::string::npos || index == 0){
		std::cout << "fail" << std::endl;
	}

	std::cout << "user_id" << decode_data.substr(0, index) << std::endl;

	std::cout << decode_data.substr(index +1, decode_data.length() - index - 1) << std::endl;
	return 0;
}

