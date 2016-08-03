#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>
#include <fstream>
#include <json/json.h>

#include <network_config.pb.h>

int main(int argc, char* argv[])
{
    Json::Value root;
    Json::Reader reader;
	network_config config;
    std::fstream jsonf("network_config.json", std::ios::binary|std::ios::in);
	std::fstream fpbuf("network_config.dat", std::ios::binary|std::ios::out|std::ios::trunc);
    if(!jsonf.is_open()){
         std::cout << "network_config.json open failed!" << std::endl;
         return 1;
    }
	if(!reader.parse(jsonf,root)){
		std::cout << "parse json failed" << std::endl;
	}

	config.set_port(root["port"].asInt());
	config.set_max_conn(root["max_conn"].asInt());
	config.set_max_overstock(root["max_overstock"].asInt());
	config.set_max_keepalive_timeout(root["max_keepalive_timeout"].asInt());
	config.set_is_open_buffer_allocator(root["is_open_buffer_allocator"].asBool());
	config.set_buffer_allocator_cache_count(root["buffer_allocator_cache_count"].asInt());



	config.SerializeToOstream(&fpbuf);


	std::cout << "network_config.dat build successfuly!" << std::endl;

    
    return 0;
}
