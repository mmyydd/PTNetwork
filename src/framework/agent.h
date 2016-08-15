#ifndef _AGENT_INCLUDED_H_
#define _AGENT_INCLUDED_H_


struct pt_agent;
struct pt_agent_node;

enum agent_node_connect_state
{
	AGENT_STATE_CONNECT_FAILED,
	AGENT_STATE_CONNECTED,
	AGENT_STATE_DISCONNECTED,
};

typedef void (*pt_agent_notify_node_state)(struct pt_agent *agent,
		struct pt_agent_node *node, enum agent_node_connect_state state);

typedef void (*pt_agent_notify_state)(struct pt_agent *agent, qboolean is_connected);

typedef void (*pt_agent_notify_user_connection)(struct pt_agent *agent, 
		struct pt_sclient *user, qboolean is_connect);
#define SERVER_ID_SPLIT 1000
struct pt_agent_node
{
	struct pt_agent_node *next;
	struct pt_agent *agent;
	char name[64];
	char host[64];
	qboolean is_pipe;
	uint16_t port;
	uint32_t server_id;
	struct pt_client *conn;
};

struct pt_agent
{
	struct pt_agent_node *nodes;
	uv_timer_t timer;
	qboolean is_active;
	qboolean is_connected;
	struct pt_server *server;


	pt_agent_notify_node_state notify_node_state;
	pt_agent_notify_state notify_state;

	pt_agent_notify_user_connection notify_user_connection;
};

struct pt_agent *pt_agent_new();
void pt_agent_free(struct pt_agent *agent);

void pt_agent_add_server(struct pt_agent *agent, const char *name, uint32_t server_id, const char *host, uint16_t port, qboolean is_pipe);

qboolean pt_agent_startup(struct pt_agent *agent, const char *ip, uint16_t port);
void pt_agent_shutdown(struct pt_agent *agent);

void pt_agent_send_to_all(struct pt_agent *agent, struct pt_buffer *buff);
#endif
