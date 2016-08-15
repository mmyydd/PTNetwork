#ifndef _AGENT_INCLUDED_H_
#define _AGENT_INCLUDED_H_


struct pt_agent;
struct pt_agent_node;

enum agent_node_connect_state
{
	AGENT_STATE_FAILED,
	AGENT_STATE_CONNECTED,
	AGENT_STATE_DISCONNECTED,
};

typedef void (*pt_agent_notify_node_state)(struct pt_agent *agent,
		struct pt_agent_node *node, enum agent_node_connect_state state);

typedef void (*pt_agent_notify_state)(struct pt_agent *agent, qboolean is_connected);

#define SERVER_ID_SPLIT 1000
struct pt_agent_node
{
	struct pt_agent_node *next;
	struct pt_agent *agent;
	char name[64];
	char host[64];
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
};



#endif
