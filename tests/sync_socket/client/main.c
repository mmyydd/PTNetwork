#include <string.h>
#include <ptnetwork.h>
#include <pthread.h>


struct pt_sync_client sync_client;
uint32_t call_count =0;
pthread_t thread;


void *thread_print(void *param)
{
	while(true)
	{
		fprintf(stderr, "call count:%u\n", call_count);
		usleep(1000 * 1000);
	}

	return NULL;
}
int main()
{
	int r;
	struct net_header hdr;
	struct pt_buffer *buff;
	const char *str_helloworld;
	struct buffer_reader reader;
	pt_sync_client_init(&sync_client);
	pt_sync_client_set_connect_pipe(&sync_client, "/var/tmp/ptnetwork_tserver.sock");

	sync_client.auto_reconnect = true;

	r = pt_sync_client_real_connect(&sync_client);

	if( r != pt_sync_err_noerr)
	{
		fprintf(stderr, "pt_sync_client_real_connect failed    %d %s\n", r, strerror(sync_client._errno));
		return 1;
	}

	str_helloworld = "1234567890123456789013123123123123";

	pthread_create(&thread, NULL, thread_print, NULL);
	
	while(true)
	{
		hdr = pt_create_nethdr(ID_TRANSMIT_JSON);
		buff = pt_create_package(hdr, str_helloworld,strlen(str_helloworld) + 0x1);
		

		r = pt_sync_client_send_packet(&sync_client, buff);
		if( r != pt_sync_err_noerr)
		{
			fprintf(stderr, "send packet failed\n");
			return 1;
		}

		buff = NULL;
		r = pt_sync_client_recv_packet(&sync_client, &buff);

		if( r != pt_sync_err_noerr)
		{
			fprintf(stderr, "receive packet failed\n");
			return 1;
		}	
		buffer_reader_init(&reader, buff);
		buffer_reader_read(&reader, &hdr, sizeof(hdr));

		call_count++;

		//fprintf(stderr,"receive package:%s\n", buffer_reader_cur_pos(&reader));

		pt_buffer_free(buff);
		
		//usleep(10 * 1000);
	}
	

	return 0;
}
