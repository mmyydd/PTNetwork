#include <common/common.h>
#include "service.hpp"


pt_service::pt_service()
{
}

pt_service::~pt_service()
{

}



qboolean pt_service::on_accept_cb(struct pt_sclient *user)
{
	pt_service_node *node = new pt_service_node();
	node->node_id = user->id;
}

void pt_service::on_read_cb(struct pt_sclient *user, struct pt_buffer *buff)
{

}
void pt_service::on_disconnect_cb(struct pt_sclient *user)
{

}
