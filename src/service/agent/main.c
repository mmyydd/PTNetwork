#include <ptframework/common/common.h>
#include <ptframework/file.h>
#include <network_config.pb-c.h>
#include <agent_servers.pb-c.h>
#include "agent.h"


uv_loop_t *uv_loop;
NetworkConfig *networkConfig;
struct pt_agent *g_agent;

qboolean load_network_config()
{ 
    uint32_t buf_len = 0;
    unsigned char *buf;
	
	if(readfile("network_config.dat", &buf, &buf_len) == false)
	{
		printf("can not open network_config.dat\n");
		return false;
	}
    
    networkConfig = network_config__unpack(NULL, buf_len, buf);
    
    if(networkConfig == NULL){
        printf("network config unpack failed\n");
        return false;
    }
    
    free(buf);
    return true;
}

qboolean load_cluster_servers()
{
    uint32_t bufsize;
	AgentServers *agentServers;
	struct pt_cluster *cluster = NULL;
	unsigned char* servers_buffer;
    size_t i;
	if(readfile("agent.dat", &servers_buffer, &bufsize) == false)
	{
		printf("can not open servers.dat\n");
		return false;
	}
    
    agentServers = agent_servers__unpack(NULL, bufsize, servers_buffer);
     
    if(!agentServers){
        printf("parse servers.dat failed\n");
        free(servers_buffer);
        return false;
    }
	
	for( i = 0; i < agentServers->n_servers;i++)
	{
		AgentServerNode *be = agentServers->servers[i];

		pt_agent_add_server(g_agent,be->server_name, be->server_id,
				be->address,be->port, be->is_pipe, be->need_login);
	}

	agent_servers__free_unpacked(agentServers, NULL);
    
    free(servers_buffer);
    return true;
}

void print_log (const char *message, const char *function, const char *file, int line)
{
	printf("%s\n",message);
}
void notify_agent_state(struct pt_agent *agent, qboolean is_connected)
{
	printf("agent status:%d\n", is_connected);
}

void notify_node_state(struct pt_agent *agent, struct pt_agent_node *node,
		enum agent_node_connect_state state)
{
	printf("node state:%s  %d\n", node->name, node->port);
}
int main(int argc, const char * argv[]) {

    set_error_report(ERROR_LEVEL_LOG, print_log);	
    uv_loop = uv_default_loop();
	printf("load network config ....\n");
	if(!load_network_config()){
        printf("load network configure failed..\n");
        return 1;
    }

	printf("load network config done...\n");
	g_agent = pt_agent_new();

	g_agent->notify_state = notify_agent_state;
	g_agent->notify_node_state = notify_node_state;

	printf("load cluster servers\n");

	if(!load_cluster_servers()) {
		printf("load cluster server failed..\n");
		return 1;
	}
	printf("load cluster servers done...\n");

	printf("agent server starting...\n");
	g_agent->server->number_of_max_connected = networkConfig->number_of_conn;

	printf("listen:%d\n", networkConfig->port);	
	if(pt_agent_startup(g_agent,"0.0.0.0",networkConfig->port) == false)
	{
		printf("can't not start agent server \n");
		return 1;
	}

	printf("agent server start success...\n");
	printf("enter uv_run....\n");

    uv_run(uv_loop, UV_RUN_DEFAULT);

	printf("leave uv_run....\n");

	pt_agent_free(g_agent);

	printf("free agent instalce\n");
	return 0;
}
