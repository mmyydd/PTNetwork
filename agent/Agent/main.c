#include "common.h"

uv_loop_t *uv_loop;

NetworkConfig *networkConfig;

qboolean load_network_config()
{
    
    uint32_t buf_len = 0;
    unsigned char *buf = file_get_buffer("network_config.dat",&buf_len);
    if(!buf){
        printf("load network config failed\n");
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

	unsigned char* servers_buffer = file_get_buffer("servers.dat", &bufsize);
    
    if(!servers_buffer){
        printf("can't open servers.dat\n");
        return false;
    }
    
    servers = backend_c__unpack(NULL, bufsize, servers_buffer);
    
    
    if(!servers){
        printf("parse servers.dat failed\n");
        free(servers_buffer);
        return false;
    }
    
    
    
    backend_c__free_unpacked(servers, NULL);
    
    free(servers_buffer);
    return true;
}

qboolean server_configure()
{
    if(!load_network_config()){
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




int main(int argc, const char * argv[]) {

    if(!server_configure()){
        printf("server configure failed\n");
        return 0;
    }
    
    uv_loop = uv_default_loop();
    
    
    server_startup();
    server_run();
    
    uv_loop_close(uv_loop);
    
    
    printf("server stoped\n");
    return 0;
}
