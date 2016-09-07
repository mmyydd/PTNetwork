#ifndef _SERVICE_INCLUDED_H_
#define _SERVICE_INCLUDED_H_



struct pt_service_user_info
{
	uint64_t node_id;
	uint64_t user_id;
};

class pt_service
{
private:
	struct pt_server *m_sock;

	static qboolean on_accept_cb(struct pt_sclient *user);
	static void on_read_cb(struct pt_sclient *user, struct pt_buffer *buff);
	static void on_disconnect_cb(struct pt_sclient *user);


private:

};


#endif
