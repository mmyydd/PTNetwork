#ifndef _SERVICE_INCLUDED_H_
#define _SERVICE_INCLUDED_H_
#include <common/common.h>
#include <coroutine.h>
#include <json/json.h>
#include <iostream>
#include <string>
#include <db_intr/db_intr.hpp>
#include "db_ext.hpp"

class service;
class service_handler
{
private:
	service &m_service;
	uint64_t m_network_id;
protected:
	service_handler(service &service, uint64_t network_id);
	virtual ~service_handler();


	void send(uint16_t package_id, unsigned char *data, uint32_t length);
	void sendJson(Json::Value &root);
};

class service
{
public:
	service();
	virtual ~service();

	void init();
	void startup();
	void run();
	void shutdown();

	void process_login(uint64_t user_id, Json::Value &root);
	void process_logout(uint64_t user_id, Json::Value &root);

	db_ext *db;
	struct pt_coroutine *m_server;

private:
	static void on_new_connection(struct pt_coroutine *routine, struct pt_coroutine_user *user);
	static void on_disconnected_connection(struct pt_coroutine *routine, struct pt_coroutine_user *user);
	static void on_received(struct pt_coroutine *routine, 
		struct pt_coroutine_user *user, struct net_header hdr, 
		struct buffer_reader *reader);
};

enum service_state
{
	STATE_A,
	STATE_B,
	STATE_C,
	STATE_D
};

class service_handler_login : public service_handler, public db_query_notify
{
protected:
	service *m_service;
	uint64_t m_net_id;
	Json::Value m_request;

public:
	service_handler_login(service *service);
	virtual ~service_handler_login();
	
	bool decode_token(const std::string &token, uint32_t &user_id, std::string &verify);
	void set_request(uint64_t net_id, Json::Value &root);
	void run();
	
	void send_login_failed();
	void get_user_info(uint32_t user_id);
	void process_a(db_intr_handle *db_query);
	virtual void on_query_completed(db_intr_handle *db_query);
	virtual void on_query_failed(db_intr_handle *query, int reason);
};

extern service g_service;

#endif
