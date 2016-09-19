#ifndef _SERVICE_INCLUDED_H_
#define _SERVICE_INCLUDED_H_

#include <ptframework/common/common.h>
#include <ptframework/common/message.h>
#include <json/json.h>


struct pt_userinfo
{
	uint64_t node_id;
	uint64_t user_id;
};

class pt_base_service_node
{
public:
	pt_base_service_node();
	virtual ~pt_base_service_node();

	bool user_exists(uint64_t user_id);
	void add_user(uint64_t user_id);
	void remove_user(uint64_t user_id);

	struct pt_table *m_users;
};

class pt_base_service
{
public:
	pt_base_service();
	virtual ~pt_base_service();
	bool start(bool is_pipe, const char *host, uint16_t port);
	bool shutdown();
	int get_error();

protected:
	virtual void on_user_connected(struct pt_userinfo userinfo);
	virtual void on_user_disconnected(struct pt_userinfo userinfo);

	virtual void on_user_received(struct pt_userinfo userinfo, struct buffer_reader* reader,struct net_header hdr);
	virtual void on_user_receive_json(struct pt_userinfo userinfo, Json::Value &root);

	virtual void on_control_package(uint64_t node_id, struct buffer_reader *reader, struct net_header hdr);

	void on_gateway_connected(struct pt_sclient *user);
	void on_gateway_recevied(struct pt_sclient *user, struct pt_buffer *buff);
	void on_gateway_disconnect(struct pt_sclient *user);

	bool send(struct pt_userinfo userinfo, struct net_header hdr, const unsigned char *data, uint32_t length);
	bool send_json(struct pt_userinfo userinfo, Json::Value &root);

	void sendall(struct net_header hdr, const unsigned char *data, uint32_t length);
	void sendall_json(Json::Value &root);

private:
	struct pt_table *m_node_table;
	struct pt_server *m_sock;

	void receiveJsonMsg(struct pt_userinfo userinfo, struct buffer_reader *reader,
			struct net_header hdr);

	Json::Value parseJson(unsigned char *data, uint32_t length);
	static qboolean on_accept_cb(struct pt_sclient *user);
	static void on_read_cb(struct pt_sclient *user, struct pt_buffer *buff);
	static void on_disconnect_cb(struct pt_sclient *user);
	
	static void sendall_cb(struct pt_table *table, uint64_t id,
			void *data, void* user_arg);
	static void sendall_cb2(struct pt_table *table, uint64_t id,
			void *data, void* user_arg);

	static void error_notify(struct pt_server *sock, const char *msg);
};


#endif
