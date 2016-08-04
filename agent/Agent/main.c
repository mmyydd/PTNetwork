#include "common.h"
#include "listener.h"
#include "backend.h"
#include "cluster.h"
#include "dispatch.h"


uv_loop_t *uv_loop;

NetworkConfig *networkConfig;

qboolean readfile(const char *filename, unsigned char **pbuf, uint32_t *len)
{
	FILE *fp;
	unsigned char *buf;
	uint32_t length;
	fp = fopen(filename, "rb");
	if(fp == NULL) return false;

	fseek(fp,0,SEEK_END);
	length = (uint32_t)ftell(fp);
	fseek(fp,0,SEEK_SET);
	
	buf = malloc(length);
	if(fread(buf, 1, length, fp) != length)
	{
		free(buf);
		fclose(fp);
		return false;
	}

	*len = length;
	*pbuf = buf;
	fclose(fp);
	
	return true;
}

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

	unsigned char* servers_buffer;
    
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

	pt_dispatch_update(uv_loop, servers);
	pt_dispatch_active();

	backend_c__free_unpacked(servers, NULL);
    
    free(servers_buffer);
    return true;
}

qboolean server_configure()
{
	if(!load_network_config()){
        return false;
    }
	if(!load_cluster_servers()){
		return false;
	}

    return true;
}

void server_startup()
{
    listener_init(uv_loop, 20140, 5000);
    
    
    
    listener_startup();
}

void server_shutdown()
{
    listener_shutdown();
}


void server_run()
{
    uv_run(uv_loop, UV_RUN_DEFAULT);
}

void print_log (const char *message, const char *function, const char *file, int line)
{
	printf("%s\n",message);
}


int main(int argc, const char * argv[]) {

    set_error_report(ERROR_LEVEL_LOG, print_log);	
    uv_loop = uv_default_loop();

	if(!server_configure()){
        printf("server configure failed\n");
        return 0;
    }
   
    server_startup();
    server_run();
    
    uv_loop_close(uv_loop);
    
    
    printf("server stoped\n");
    return 0;
}
