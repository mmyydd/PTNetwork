#include <stdio.h>
#include <ptnetwork.h>

uv_loop_t *uv_loop;

	uv_timer_t timer;

struct pt_client *conn;
const uint32_t RC4Key[4] = {0x42970C86, 0xA0B3A057, 0x51B97B3C, 0x70F8891E};

void on_timer(uv_timer_t *tm);
void on_connect_notify(struct pt_client *conn,enum pt_client_state state)
{
	if(state == PT_CONNECTED)
	{
		uv_timer_start(&timer, on_timer, 10, 10);
		printf("connected\n");
	}

	if(state == PT_CONNECT_FAILED)
	{
		printf("connect failed\n");
	}

	if(state == PT_NO_CONNECT)
	{
		printf("client not connect\n");
	}
}

void on_receive(struct pt_client *user, struct pt_buffer *buff)
{
	struct buffer_reader reader;
	struct net_header header;
	unsigned char *data;
	uint32_t length;
	char *s;

	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &header, sizeof(header));
	data = buffer_reader_cur_pos(&reader);
	length = buffer_reader_over_size(&reader);
	s = malloc(length + 1);
	bzero(s, length + 1);
	memcpy(s, data, length);
	printf("receive:id:%d    %s\n", header.id,  s);

	free(s);
}

void on_disconnect(struct  pt_client *conn)
{
	uv_timer_stop(&timer);
	printf("server disconnected....\n");	
}

uint32_t serial = 0;

void postA()
{

	char text[64];
	struct pt_buffer *buff = NULL;
	struct net_header header;
	header = pt_create_nethdr(1000 + ID_TRANSMIT_JSON);

	sprintf(text,"{\"action\":\"hello_world\"}");
	buff = pt_create_encrypt_package(&conn->encrypt_ctx,&conn->serial, header, (unsigned char*)&text, strlen(text) +1);

	pt_client_send(conn, buff);
}


void postB()
{
	char text[64];
	struct pt_buffer *buff = NULL;
	struct net_header header;
	header = pt_create_nethdr(1000 + ID_USER_PACKET_ENUM);
		
	sprintf(text,"{\"action\":\"hello_world\"}");
	buff = pt_create_encrypt_package(&conn->encrypt_ctx,&conn->serial, header, (unsigned char*)&text, strlen(text) +1);
	
	pt_client_send(conn, buff);

}
void on_timer(uv_timer_t *tm)
{
	if(conn->state == PT_CONNECTED)
	{
		postA();
		postB();
	}
}


int main(int argc ,char *argv[])
{
	uv_loop = uv_default_loop();

	conn = pt_client_new();

	uv_timer_init(uv_loop, &timer);

	pt_client_init(uv_loop, conn, on_connect_notify, on_receive, on_disconnect);

	pt_client_set_encrypt(conn, RC4Key);
	pt_client_connect(conn,"127.0.0.1",27015); 

	uv_run(uv_loop, UV_RUN_DEFAULT);
	return 0;
}
