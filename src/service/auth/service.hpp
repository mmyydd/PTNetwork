#ifndef _SERVICE_INCLUDED_H_
#define _SERVICE_INCLUDED_H_
#include "../framework/coroutine.h"

class service
{
public:
	service();
	virtual ~service();

	bool start(const char *host, uint16_t port, bool pipe);
	


private:
	static void on_new_connection(struct pt_coroutine *routine, struct pt_coroutine_user *user);
	static void on_disconnected_connection(struct pt_coroutine *routine, struct pt_coroutine_user *user);
	static void on_received(struct pt_coroutine *routine, 
		struct pt_coroutine_user *user, struct net_header hdr, 
		struct buffer_reader *reader);

	struct pt_coroutine *m_server;
};

#endif
