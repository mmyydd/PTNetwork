#include <stdio.h>
#include <agent_servers.pb.h>
#include <iostream>
#include <string>
#include <json/json.h>
#include <fstream>
const char *network_config_json = "agent.json";
 
using namespace std;
agent_servers servers;

bool check_item(Json::Value &root)
{

	return true;
}


void add_node(Json::Value &root)
{
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



	for(int i =0; i < root.size(); i++)
	{
		agent_server_node *node = servers.add_servers();

		node->set_server_name(root[i]["server_name"].asString());
		node->set_is_pipe(root[i]["is_pipe"].asBool());
		node->set_address(root[i]["address"].asString());
		node->set_port(root[i]["port"].asInt());
		node->set_server_id(root[i]["server_id"].asInt());
		node->set_need_login(root[i]["need_login"].asBool());

		std::cout << node->DebugString() << std::endl;
	}


	fstream output("agent.dat",ios::trunc | ios::out | ios::binary);
	servers.SerializeToOstream(&output);

	cout << "Serialize Success" << endl;
	return 0;
}
