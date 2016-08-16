#include <uv.h>
#include <unistd.h>
static char output[4096];
static int output_used; 
static char exepath[1024];
static size_t exepath_size = 1024;
static char* args[3];
uv_process_t process;
static void on_exit(uv_process_t *process, int64_t exit_status, int term_signal)
{
	printf("subprocess exit....\n");
}
static void on_alloc(uv_handle_t* handle,
		size_t suggested_size,
		uv_buf_t* buf) {

	  buf->base = output + output_used;
	    buf->len = sizeof(output)- output_used;
}

int main(int argc, char *argv[])
{
	int r;
	uv_pipe_t input,output;
	uv_process_options_t options = {0};

	uv_stdio_container_t stdios[3];

	printf("process start %d\n", argc);
	
	if(argc > 2 && strcmp(argv[1], "spawn") == 0)
	{
		printf("sub process print\n");
		sleep(5);
		return 0;
	}

	printf("uv_begin\n");
	uv_pipe_init(uv_default_loop(), &output, 0);
	uv_pipe_init(uv_default_loop(), &input, 0);
	args[0] = exepath;
	args[1] = "spawn";
	args[2] = NULL;
	printf("init options\n");
	options.exit_cb =  on_exit;
	options.stdio = stdios;
	options.stdio[0].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;
	options.stdio[0].data.stream = (uv_stream_t*)&output;
	options.stdio[1].flags = UV_CREATE_PIPE | UV_READABLE_PIPE | UV_WRITABLE_PIPE;
	options.stdio[1].data.stream = (uv_stream_t*)&input;
	options.stdio[2].flags = UV_CREATE_PIPE | UV_READABLE_PIPE | UV_WRITABLE_PIPE;
	options.stdio[2].data.stream = (uv_stream_t*)&input;
	options.stdio_count = 2;
	r = uv_exepath(exepath, &exepath_size);
	printf("exepath\n");
	exepath[exepath_size] = '\0';
	options.args = args;
	options.file = exepath;
	options.flags = 0;

	printf("start spawn\n");
	r  =uv_spawn(uv_default_loop(), &process, &options);
	if(r != 0 )
	{
		printf("uv_spawn error:%s\n", uv_strerror(r));
	}

	printf("uv_spawn:%d\n", r);

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);


	printf("master process exit...\n");

	return 0;
}
