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
		uv_timer_start(&timer, on_timer, 1000, 1000);
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
	uint32_t serial;
	unsigned char *data;
	uint32_t length;


	buffer_reader_init(&reader, buff);
	buffer_reader_read(&reader, &header, sizeof(header));
	//buffer_reader_read(&reader, &serial, sizeof(uint32_t));
	data = buffer_reader_cur_pos(&reader);
	length = buffer_reader_over_size(&reader);
	printf("receive:%s\n", (const char*)data);
}

void on_disconnect(struct  pt_client *conn)
{
	uv_timer_stop(&timer);
	printf("server disconnected....\n");	
}

uint32_t serial = 0;
void on_timer(uv_timer_t *tm)
{
	char text[32];
	struct pt_buffer *buff = NULL;
	struct net_header header;
	if(conn->state == PT_CONNECTED)
	{
		header = pt_create_nethdr(1002);
		
		sprintf(text,"string:%u",serial);
		buff = pt_create_encrypt_package(&conn->encrypt_ctx,&conn->serial, header, (unsigned char*)&text, strlen(text) +1);

		printf("send encrypt package:%d   %u\n",pt_client_send(conn, buff), ++serial);
	}
}


int main(int argc ,char *argv[])
{
	uv_loop = uv_default_loop();

	conn = pt_client_new();

	uv_timer_init(uv_loop, &timer);

	//uv_timer_start(&timer, on_timer, 1, 1);


	pt_client_init(uv_loop, conn, on_connect_notify, on_receive, on_disconnect);

	pt_client_set_encrypt(conn, RC4Key);
	pt_client_connect(conn,"127.0.0.1",20140); 

	uv_run(uv_loop, UV_RUN_DEFAULT);
	return 0;
}
