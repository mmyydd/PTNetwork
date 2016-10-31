#ifndef _PT_SYNC_CLIENT_H_INCLUDED_
#define _PT_SYNC_CLIENT_H_INCLUDED_

#include "common.h"
#include <sys/un.h>
enum pt_sync_error
{
	pt_sync_err_noerr,
	pt_sync_err_getaddrinfo_fail,
	pt_sync_err_find_domain_ip_fail,
	pt_sync_err_no_set_adr,
	pt_sync_err_already_connected,
	pt_sync_err_alloc_socket_fail,
	pt_sync_err_connect_fail,
	pt_sync_err_no_connect,
	pt_sync_err_disconnected,
	pt_sync_err_incorret_header,
};


struct pt_sync_client
{
	qboolean auto_reconnect;
	qboolean is_connected;
	qboolean is_set_adr;
	qboolean is_un;

	struct sockaddr_in adr;
	struct sockaddr_un un_adr;

	int _errno;
	int fd;
};



void pt_sync_client_init(struct pt_sync_client *sync_client);
void pt_sync_client_close(struct pt_sync_client *sync_client);
int pt_sync_client_set_connect(struct pt_sync_client *sync_client, const char *host, uint16_t port);
void pt_sync_client_set_connect_pipe(struct pt_sync_client *sync_client, const char *file);

int pt_sync_client_real_connect(struct pt_sync_client *sync_client);
int pt_sync_client_disconnect(struct pt_sync_client *sync_client);
int pt_sync_client_send(struct pt_sync_client *sync_client, unsigned char *data, uint32_t length);
int pt_sync_client_send_packet(struct pt_sync_client *sync_client, struct pt_buffer *buffer);
int pt_sync_client_recv(struct pt_sync_client *sync_client, unsigned char *buffer, uint32_t *length);
int pt_sync_client_recv_block(struct pt_sync_client *sync_client, unsigned char *buffer, uint32_t length);
int pt_sync_client_recv_packet(struct pt_sync_client *sync_client, struct pt_buffer **buffer);



#endif
