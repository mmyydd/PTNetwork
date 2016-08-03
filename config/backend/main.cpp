#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>
#include <fstream>
#include <json/json.h>

#include <backend.pb.h>

int main(int argc, char* argv[])
{
    Json::Value root;
    Json::Reader reader;

	backend_c backends;


    std::fstream jsonf("backend.json", std::ios::binary|std::ios::in);
	std::fstream fpbuf("backend.dat", std::ios::binary|std::ios::out|std::ios::trunc);
    if(!jsonf.is_open()){
         std::cout << "network_config.json open failed!" << std::endl;
         return 1;
    }

	if(!reader.parse(jsonf,root)){
		std::cout << "parse json failed" << std::endl;
	}

	

	for(int i = 0; i < root.size(); i ++)
	{
		backend *server = backends.add_servers();
		server->set_server_name(root[i]["server_name"].asString());
		server->set_server_id(root[i]["server_id"].asInt());

		server->set_server_type(root[i]["server_type"].asBool());
		server->set_server_host(root[i]["server_host"].asString());
		server->set_server_port(root[i]["server_port"].asInt());
	}

	backends.SerializeToOstream(&fpbuf);

	std::cout << "backend.dat build successfuly!" << std::endl;

    
    return 0;
}
