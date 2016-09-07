#include <stdio.h>
#include <database_config.pb.h>
#include <iostream>
#include <string>
#include <json/json.h>
#include <fstream>
const char *network_config_json = "database_config.json";
 
using namespace std;

database_config config;

bool check_item(Json::Value &root)
{
	if(!root.isMember("name")){
		printf("%s is not member\n", "name");
		return false;
	}

	if(root["name"].type() != Json::stringValue){
		printf("%s is invalid type\n", "name");
		return false;
	}

	if(!root.isMember("address")){
		printf("%s is not member\n", "address");
		return false;
	}

	if(root["address"].type() != Json::stringValue){
		printf("%s is invalid type\n", "address");
		return false;
	}

	if(!root.isMember("port")){
		printf("%s is not member\n", "port");
		return false;
	}

	if(root["port"].type() != Json::intValue){
		printf("%s is invalid type\n", "port");
		return false;
	}


	if(!root.isMember("username")){
		printf("%s is not member\n", "username");
		return false;
	}

	if(root["username"].type() != Json::stringValue){
		printf("%s is invalid type\n", "username");
		return false;
	}
	if(!root.isMember("password")){
		printf("%s is not member\n", "password");
		return false;
	}

	if(root["password"].type() != Json::stringValue){
		printf("%s is invalid type\n", "password");
		return false;
	}


	if(!root.isMember("dbname")){
		printf("%s is not member\n", "dbname");
		return false;
	}

	if(root["dbname"].type() != Json::stringValue){
		printf("%s is invalid type\n", "dbname");
		return false;
	}

	return true;
}


void add_node(Json::Value &root)
{
	database_node *node = config.add_databases();
	
	node->set_name(root["name"].asString());
	node->set_address(root["address"].asString());
	node->set_port(root["port"].asInt());
	node->set_username(root["username"].asString());
	node->set_password(root["password"].asString());
	node->set_dbname(root["dbname"].asString());
}
int main(int argc, char *argv[])
{
	fstream file(network_config_json, ios::in | ios::binary);
	Json::Value root;
	Json::Reader reader;
	

	if(!file.is_open()){
		cout << network_config_json << " file not found" << endl;
		return 0;
	}
	

	if(!reader.parse(file, root)){
		cout << "parse json failed" << endl;
		return 0;
	}


	for(int i = 0; i < (int)root.size();i++)
	{
		if(check_item(root[i]))
		{
			add_node(root[i]);
		}
		else
		{
			printf("check failed\n");
			exit(1);
		}
	}

	std::cout << config.DebugString() << std::endl;

	fstream output("database_config.dat",ios::trunc | ios::out | ios::binary);
	config.SerializeToOstream(&output);

	cout << "Serialize Success" << endl;
	return 0;
}
