#include <common/common.h>
#include "main.h"
#include "signal.h"



void signal_send_quit()
{
	int uPid;
	int r;
	FILE *fp = fopen(PID_FILE, "rb");

	if(fp == NULL)
	{
		printf("open pid file failed\n");
		exit(0);
	}
	fscanf(fp, "%d", &uPid);
	fclose(fp);

	r = uv_kill(uPid,SIGTERM); 

	if( r == 0 )
	{
		printf("send SIGTERM ok\n");
	}
	else
	{
		printf("send SIGTERM failed\n");
	}
}

int signal_main(int argc, char *argv[])
{
	if(strcmp(signal_cmd, "quit") == 0)
	{
		signal_send_quit();
	}
	return 0;
}
