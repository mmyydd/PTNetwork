#include <common.h>
#include <agent.h>
#include <backend.pb-c.h>
#include <network_config.pb-c.h>
#include <file.h>

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
    BackendC *servers;
	struct pt_cluster *cluster = NULL;
	unsigned char* servers_buffer;
    size_t i;
	if(readfile("servers.dat", &servers_buffer, &bufsize) == false)
	{
		printf("can not open servers.dat\n");
		return false;
	}
    
    servers = backend_c__unpack(NULL, bufsize, servers_buffer);
     
    if(!servers){
        printf("parse servers.dat failed\n");
        free(servers_buffer);
        return false;
    }
	
	for( i = 0; i < servers->n_servers;i++)
	{
		Backend *be = servers->servers[i];

		pt_agent_add_server(g_agent,be->server_name, be->server_id,
				be->server_host,be->server_port, be->server_type);
	}

	backend_c__free_unpacked(servers, NULL);
    
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
	g_agent->server->number_of_max_connected = networkConfig->max_conn;

	printf("listen:%d\n", networkConfig->port);	
	if(pt_agent_startup(g_agent,"0.0.0.0",networkConfig->port) == false)
	{
		printf("agent startup\n");
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
