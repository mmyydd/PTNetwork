#include <common/common.h>
#include <commander.h>
#include "main.h"
#include "common/message.h"
#include "netchan.h"
#include "query.h"

static void on_sigterm(uv_signal_t *handle, int signal)
{
	netchan_break_operation();
	query_queue_break_operation();
	
	uv_signal_stop(&sigterm);
	sigint_stop();
}

//网络通信进程的入口
int listener_main(int argc, char *argv[])
{
	uv_disable_stdio_inheritance();
	uv_signal_start(&sigterm, on_sigterm, SIGTERM);
	sigint_start();

	netchan_pre_operation();
	query_queue_pre_operation();

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	netchan_post_operation();
	query_queue_post_operation();
	return 0;
}
