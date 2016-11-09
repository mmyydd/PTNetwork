#include "common.h"
#include "sync_client.h"
#include "buffer.h"
#include "packet.h"

#include <sys/socket.h>
#include <sys/un.h>

void pt_sync_client_init(struct pt_sync_client *sync_client)
{
	bzero(sync_client, sizeof(struct pt_sync_client));

	sync_client->fd = -1;
}


void pt_sync_client_close(struct pt_sync_client *sync_client)
{
	if(sync_client->fd != -1)
	{
		close(sync_client->fd);
		sync_client->fd = -1;
		sync_client->is_connected = false;
	}
}


int pt_sync_client_set_connect(struct pt_sync_client *sync_client, const char *host, uint16_t port)
{
	struct addrinfo hints, *servinfo, *p;
	int rv;
	qboolean done;
	struct sockaddr_in adr;
	char str_port[10];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	servinfo = NULL;

	sprintf(str_port, "%u", port);
	if ((rv = getaddrinfo( host, str_port,  &hints, &servinfo)) != 0) {
		sync_client->_errno = errno;
		return pt_sync_err_getaddrinfo_fail;
	}

	bzero(&adr, sizeof(adr));

	for(p = servinfo; p != NULL; p = p->ai_next) {
		memcpy(&adr, p->ai_addr, sizeof(struct sockaddr_in));
		break;
	}
	
	if(p == NULL)
	{
		freeaddrinfo(servinfo);
		return pt_sync_err_find_domain_ip_fail;
	}

	memcpy( &sync_client->adr, &adr, sizeof( adr ) );

	sync_client->is_un = false;
	sync_client->is_set_adr = true;

	freeaddrinfo(servinfo);

	return pt_sync_err_noerr;
}

void pt_sync_client_set_connect_pipe(struct pt_sync_client *sync_client, const char *file)
{
	sync_client->un_adr.sun_family = PF_UNIX;
	strcpy(sync_client->un_adr.sun_path, file);

	sync_client->is_un = true;
	sync_client->is_set_adr = true;
}


int pt_sync_client_real_connect(struct pt_sync_client *sync_client)
{
	int options;

	if(sync_client->is_set_adr == false)
	{
		return pt_sync_err_no_set_adr;
	}

	if(sync_client->is_connected == true)
	{
		return pt_sync_err_already_connected;
	}
	if(sync_client->is_un)
	{
		sync_client->fd = socket(PF_UNIX, SOCK_STREAM, 0);
	}
	else
	{
		sync_client->fd = socket(AF_INET, SOCK_STREAM, 0);
	}

	if(sync_client->fd == -1)
	{
		sync_client->_errno = errno;
		return pt_sync_err_alloc_socket_fail;
	}

	if(sync_client->is_un)
	{
		options = connect(sync_client->fd, (struct sockaddr *)&sync_client->un_adr, sizeof(sync_client->un_adr));
	}
	else
	{
		options = connect(sync_client->fd, (struct sockaddr *)&sync_client->adr, sizeof(sync_client->adr));
	}

	if(options != 0)
	{
		sync_client->_errno = errno;

		close(sync_client->fd);
		sync_client->fd = -1;

		return pt_sync_err_connect_fail;
	}

	
	//强制数据发送出去
	//因为代码用于RPC 需要及时响应 所以禁用nagle 并且设置cork为0强行发送数据
	options = 1;
	setsockopt(sync_client->fd, SOL_TCP, TCP_NODELAY, &options, sizeof(options));

	options = 0;
	setsockopt(sync_client->fd, SOL_TCP, TCP_CORK, &options, sizeof(options));

	sync_client->is_connected = true;
	return pt_sync_err_noerr;
}

int pt_sync_client_disconnect(struct pt_sync_client *sync_client)
{
	if(sync_client->is_connected == false)
	{
		return pt_sync_err_no_connect;
	}
	
	close(sync_client->fd);
	sync_client->fd = -1;

	return pt_sync_err_noerr;
}


int pt_sync_client_send(struct pt_sync_client *sync_client, unsigned char *data, uint32_t length)
{
	int ret;
	uint32_t completed_size;
	if(sync_client->is_connected == false)
	{
		ret = pt_sync_client_real_connect(sync_client);

		if(ret != pt_sync_err_noerr)
		{
			return ret;
		}
	}
	

	completed_size = 0;

	do
	{
		ret = send(sync_client->fd, data, length, 0);

		if(ret <= 0)
		{
			sync_client->_errno = errno;
			pt_sync_client_disconnect(sync_client);
			return pt_sync_err_disconnected;
		}

		completed_size += ret;

		if(completed_size == length)
			break;

	}while(true);
	

	return pt_sync_err_noerr;
}


int pt_sync_client_send_packet(struct pt_sync_client *sync_client, struct pt_buffer *buffer)
{
	int ret;
	
resend:
	ret = pt_sync_client_send(sync_client, buffer->buff, buffer->length);

	if(ret == pt_sync_err_noerr)
	{
		pt_buffer_free(buffer);
		return ret;
	}
	
	if(ret == pt_sync_err_disconnected && sync_client->auto_reconnect)
	{
		ret = pt_sync_client_real_connect(sync_client);

		if(ret == pt_sync_err_noerr)
		{
			goto resend;
		}
	}

	pt_buffer_free(buffer);

	return ret;
}



int pt_sync_client_recv(struct pt_sync_client *sync_client, unsigned char *buffer, uint32_t *length)
{
	uint32_t _length = *length;
	int ret;

	if(sync_client->is_connected == false)
	{
		return pt_sync_err_no_connect;
	}

	ret = recv(sync_client->fd, buffer, _length, 0);

	if(ret <= 0)
	{
		sync_client->_errno = errno;
		pt_sync_client_disconnect(sync_client);
		return pt_sync_err_disconnected;
	}

	*length = ret;

	return pt_sync_err_noerr;
}

int pt_sync_client_recv_block(struct pt_sync_client *sync_client, unsigned char *buffer, uint32_t length)
{
	unsigned char *data;
	uint32_t write_size;
	uint32_t over_size;
	int ret;

	if(sync_client->is_connected == false)
	{
		return pt_sync_err_no_connect;
	}

	write_size = 0;

	do
	{
		over_size = length - write_size;
		data = &buffer[write_size];

		ret = pt_sync_client_recv(sync_client, data, &over_size);
		if(ret != pt_sync_err_noerr) 
		{
			break;
		}

		write_size += over_size;

		if(write_size == length)
			break;

	}while(true);

	return ret;
}


int pt_sync_client_recv_packet(struct pt_sync_client *sync_client,  struct pt_buffer **buffer)
{
	struct pt_buffer *buff;
	int ret;
	struct net_header hdr;
	unsigned char *data;
	uint32_t over_size;
	
	if(sync_client->is_connected == false)
	{
		return pt_sync_err_no_connect;
	}
	
	ret = pt_sync_client_recv_block(sync_client, (unsigned char *)&hdr, sizeof(hdr));

	if(ret != pt_sync_err_noerr)
	{
		return ret;
	}

	if(hdr.magic != PACKET_MAGIC)
	{
		return pt_sync_err_incorret_header;
	}

	if(hdr.length > pt_max_pack_size)
	{
		return pt_sync_err_incorret_header;
	}

	buff = pt_buffer_new(hdr.length);
	pt_buffer_write(buff, &hdr, sizeof(hdr));
	
	data = &buff->buff[sizeof(struct net_header)];
	over_size = hdr.length - sizeof(struct net_header);

	ret = pt_sync_client_recv_block(sync_client, data, over_size);

	if(ret != pt_sync_err_noerr)
	{
		pt_buffer_free(buff);
		return ret;
	}

	buff->length = hdr.length;
	*buffer = buff;

	return ret;
}
