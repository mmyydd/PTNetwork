#include "service.hpp"

struct sendall_info
{
	pt_base_service *service;
	struct pt_sclient *curr_node;
	struct net_header hdr;
	const unsigned char *data;
	uint32_t length;
};

pt_base_service_node::pt_base_service_node()
{
	m_users = pt_table_new();
}

pt_base_service_node::~pt_base_service_node()
{
	pt_table_free(m_users);
}

void pt_base_service_node::add_user(uint64_t user_id)
{
	pt_table_insert(m_users, user_id, (void*)user_id);
}

void pt_base_service_node::remove_user(uint64_t user_id)
{
	pt_table_erase(m_users, user_id);
}

bool pt_base_service_node::user_exists(uint64_t user_id)
{
	if(pt_table_find(m_users, user_id))
	{
		return true;
	}
	return false;
}

pt_base_service::pt_base_service()
{
	m_sock = pt_server_new();

	m_node_table = pt_table_new_ex(200);

	m_sock->data= this;

	pt_server_init(m_sock, uv_default_loop(),10000,10,
			on_accept_cb, on_read_cb, on_disconnect_cb);

	m_sock->error_notify = error_notify;
}

pt_base_service::~pt_base_service()
{
	pt_server_free(m_sock);
	pt_table_free(m_node_table);

	m_sock = NULL;
}


void pt_base_service::error_notify(struct pt_server *sock, const char *msg)
{
	pt_base_service *service = (pt_base_service *)sock->data;
	fprintf(stderr,"%s\n", msg);
}

bool pt_base_service::start(bool is_pipe, const char *host, uint16_t port)
{
	qboolean ret = false;
	if(is_pipe)
	{
		ret = pt_server_start_pipe(m_sock, host);
	}
	else
	{
		ret = pt_server_start(m_sock,host, port);
	}

	return ret;
}

void pt_base_service::on_user_connected(struct pt_userinfo userinfo)
{
	fprintf(stderr, "node:%lu user:%lu connected.\n", userinfo.node_id, userinfo.user_id);
}

void pt_base_service::on_user_received(struct pt_userinfo userinfo, struct buffer_reader *reader,struct net_header hdr)
{
	fprintf(stderr, "node:%lu user:%lu receive msg:%u bytes %u.\n", userinfo.node_id, userinfo.user_id, hdr.id, buffer_reader_over_size(reader));
}

void pt_base_service::on_user_receive_json(struct pt_userinfo userinfo, Json::Value &root)
{
	fprintf(stderr, "node:%lu user:%lu receive json:%s\n", userinfo.node_id, userinfo.user_id, root.toStyledString().c_str());

	send_json(userinfo, root);
}

void pt_base_service::on_user_disconnected(struct pt_userinfo userinfo)
{
	fprintf(stderr, "node:%lu user:%lu disconnected.\n", userinfo.node_id, userinfo.user_id);
}

void pt_base_service::on_gateway_connected(struct pt_sclient *user)
{
	fprintf(stderr, "gateway connected:%lu\n",user->id);
}

void pt_base_service::receiveJsonMsg(struct pt_userinfo userinfo, struct buffer_reader *reader,struct net_header hdr)
{
	unsigned char *data;
	uint32_t length;

	Json::Value root;
	Json::Reader jsonReader;

	data = buffer_reader_cur_pos(reader);
	length = buffer_reader_over_size(reader);
	std::string jsonStr((const char*)data, length);

	if(jsonReader.parse(jsonStr, root))
	{
		on_user_receive_json(userinfo, root);
	}
}

void pt_base_service::on_control_package(uint64_t node_id, struct buffer_reader *reader)
{

}

void pt_base_service::on_gateway_recevied(struct pt_sclient *user, struct pt_buffer *buff)
{
	struct pt_userinfo userinfo;
	struct buffer_reader reader;
	struct net_header hdr;
	uint64_t user_id;
	pt_base_service_node *node = (pt_base_service_node*)user->data;

	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &hdr, sizeof(hdr));

	switch(hdr.id)
	{
		case ID_USER_CONNECTED:
			buffer_reader_read(&reader, &user_id,sizeof(user_id));
			userinfo.node_id = user->id;
			userinfo.user_id = user_id;
			node->add_user(user_id);
			on_user_connected(userinfo);
			break;
		case ID_USER_DISCONNECTED:
			buffer_reader_read(&reader, &user_id,sizeof(user_id));
			userinfo.node_id = user->id;
			userinfo.user_id = user_id;
			on_user_disconnected(userinfo);
			node->remove_user(user_id);
			break;
		case ID_TRANSMIT_JSON:
			buffer_reader_read(&reader, &user_id, sizeof(user_id));
			userinfo.node_id = user->id;
			userinfo.user_id = user_id;
			receiveJsonMsg(userinfo, &reader, hdr);
			break;
		default:
			break;
	}

	if(hdr.id >= ID_USER_PACKET_ENUM)
	{
		buffer_reader_read(&reader, &user_id, sizeof(user_id));
		userinfo.node_id = user->id;
		userinfo.user_id = user_id;
		on_user_received(userinfo, &reader, hdr);
	} else if (hdr.id >= ID_CONTROL_PACKET){
		on_control_package(user->id, &reader);
	}
}

void pt_base_service::on_gateway_disconnect(struct pt_sclient *user)
{
	fprintf(stderr, "gateway %lu disconnected\n", user->id);
}

bool pt_base_service::send(struct pt_userinfo userinfo, struct net_header hdr,
		const unsigned char *data, uint32_t length)
{
	struct pt_sclient *node = (struct pt_sclient*)pt_table_find(m_node_table, userinfo.node_id);

	if(node == NULL){
		return false;
	}

	pt_base_service_node *service_node = (pt_base_service_node *)node->data;

	if(!service_node->user_exists(userinfo.user_id)){
		return false;
	}

	struct net_header *net_hdr;
	struct pt_buffer *buff = pt_buffer_new(length + sizeof(hdr) + sizeof(uint64_t));

	pt_buffer_write(buff, &hdr, sizeof(hdr));
	pt_buffer_write(buff, &userinfo.user_id, sizeof(userinfo.user_id));
	pt_buffer_write(buff, data, length);

	net_hdr = (struct net_header *)buff->buff;
	net_hdr->length = buff->length;

	return pt_server_send(node, buff);
}

bool pt_base_service::send_json(struct pt_userinfo userinfo, Json::Value &root)
{
	struct pt_sclient *node = (struct pt_sclient*)pt_table_find(m_node_table, userinfo.node_id);

	if(node == NULL){
		fprintf(stderr, "node == null\n");
		return false;
	}

	pt_base_service_node *service_node = (pt_base_service_node *)node->data;

	if(!service_node->user_exists(userinfo.user_id)){
		fprintf(stderr, "user %lu not exists\n", userinfo.user_id);
		return false;
	}

	Json::FastWriter fastWriter;
	std::string jsonStr = fastWriter.write(root);

	struct net_header *net_hdr;
	struct pt_buffer *buff = pt_buffer_new(jsonStr.length()+ sizeof(struct net_header) + sizeof(uint64_t));
	
	struct net_header hdr = pt_create_nethdr(ID_TRANSMIT_JSON);

	pt_buffer_write(buff, &hdr, sizeof(hdr));
	pt_buffer_write(buff, &userinfo.user_id, sizeof(userinfo.user_id));
	pt_buffer_write(buff, jsonStr.c_str(), jsonStr.length());

	net_hdr = (struct net_header *)buff->buff;
	net_hdr->length = buff->length;

	return pt_server_send(node, buff);
}

void pt_base_service::sendall_cb2(struct pt_table *table, uint64_t id, void *data, void *user_arg)
{
	struct pt_userinfo userinfo;
	struct sendall_info *record;
	record = (struct sendall_info*)user_arg;

	userinfo.node_id = record->curr_node->id;
	userinfo.user_id = id;

	record->service->send(userinfo,record->hdr, record->data,record->length);
}

void pt_base_service::sendall_cb(struct pt_table *table, uint64_t id, void *data, void *user_arg)
{
	struct sendall_info *record;
	record = (struct sendall_info*)user_arg;

	struct pt_sclient *user = (struct pt_sclient *)data;
	pt_base_service_node *service_node = (pt_base_service_node*)user->data;

	record->curr_node = user;

	pt_table_enum(service_node->m_users, sendall_cb2, user_arg);
}

void pt_base_service::sendall(struct net_header hdr, const unsigned char *data, uint32_t length)
{
	struct sendall_info record;
	record.service = this;
	record.hdr = hdr;
	record.data = data;
	record.length = length;
	pt_table_enum(m_node_table, sendall_cb, &record);
}

void pt_base_service::sendall_json(Json::Value &root)
{
	Json::FastWriter fastWriter;
	struct net_header hdr = pt_create_nethdr(ID_TRANSMIT_JSON);

	std::string jsonStr = fastWriter.write(root);
	sendall(hdr, (unsigned char *)jsonStr.c_str(), jsonStr.length());
}

qboolean pt_base_service::on_accept_cb(struct pt_sclient *user)
{
	pt_base_service *service =(pt_base_service*) user->server->data;

	user->data = new pt_base_service_node();
	pt_table_insert(service->m_node_table, user->id, user);

	service->on_gateway_connected(user);
	return true;
}

void pt_base_service::on_read_cb(struct pt_sclient *user, struct pt_buffer *buff)
{
	pt_base_service *service =(pt_base_service*) user->server->data;

	service->on_gateway_recevied(user, buff);
}

void pt_base_service::on_disconnect_cb(struct pt_sclient *user)
{
	pt_base_service *service =(pt_base_service*) user->server->data;
	service->on_gateway_disconnect(user);

	pt_table_erase(service->m_node_table, user->id);

	pt_base_service_node *node = (pt_base_service_node*)user->data;
	delete node;
	user->data = NULL;
}

int pt_base_service::get_error()
{
	return m_sock->last_error;
}
