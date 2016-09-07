#include <stdio.h>
#include <network_config.pb.h>
#include <iostream>
#include <string>
#include <json/json.h>
#include <fstream>
const char *network_config_json = "network_config.json";
 
using namespace std;

network_config config;

bool check_item(Json::Value &root)
{
	if(!root.isMember("port") || !root.isMember("number_of_conn") || !root.isMember("keep_time") ||
		   	!root.isMember("is_enable_cache") || !root.isMember("cache_count"))
	{
		return false;
	}

	if(root["port"].type() != Json::intValue){
		return false;
	}

	if(root["number_of_conn"].type() != Json::intValue){
		return false;
	}
	if(root["keep_time"].type() != Json::intValue){
		return false;
	}
	if(root["is_enable_cache"].type() != Json::booleanValue){
		return false;
	}
	if(root["cache_count"].type() != Json::intValue){
		return false;
	}
	return true;
}


void add_node(Json::Value &root)
{
	int port = root["port"].asInt();
	int number_of_conn = root["number_of_conn"].asInt();
	int keep_time = root["keep_time"].asInt();
	bool is_enable_cache = root["is_enable_cache"].asBool();
	int cache_count = root["cache_count"].asInt();

	config.set_port(port);
	config.set_number_of_conn(number_of_conn);
	config.set_keep_time(keep_time);
	config.set_is_enable_cache(is_enable_cache);
	config.set_cache_count(cache_count);


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


	if(!check_item(root)){
		cout << "check item failed" << endl;
		return 0;
	}

	add_node(root);

	fstream output("network_config.dat",ios::trunc | ios::out | ios::binary);
	config.SerializeToOstream(&output);

	cout << "Serialize Success" << endl;
	return 0;
}
