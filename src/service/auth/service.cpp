#include <common.h>
#include "service.hpp"



service::service()
{
	m_server = pt_coroutine_new();
	m_server->data = this;

	pt_coroutine_init(m_server,
			on_new_connection,
			on_disconnected_connection,
			on_received);
}


service::~service()
{
	pt_coroutine_free(m_server);
}


void service::on_new_connection(struct pt_coroutine *routine,
		struct pt_coroutine_user *user)
{
	
}

void service::on_received(struct pt_coroutine *routine, 
		struct pt_coroutine_user *user, struct net_header hdr, 
		struct buffer_reader *reader)
{
}


void service::on_disconnected_connection(struct pt_coroutine *routine,
		struct pt_coroutine_user *user)
{

}
bool service::start(const char *host, uint16_t port, bool pipe)
{
	return pt_coroutine_start(m_server, pipe, host,port);
}
