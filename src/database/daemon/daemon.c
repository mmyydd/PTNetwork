#include <ptframework/common/common.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "../main.h"
#include "daemon.h"

char exepath[1024];
size_t exepath_size;

int daemon_argc;
char **daemon_argv;
uv_process_t detach_process;
int spawn_args_index = 0;
char *spawn_args[100] = {NULL};

uv_timer_t daemon_timer;
struct pt_daemon_proc *listener_proc = NULL;
struct pt_daemon_proc *logic_procs = NULL;

static void spawn_some_logic_process();
#define USE_LISTENER
#define USE_LOGIC

void pt_daemon_close_handle_cb(uv_handle_t *handle)
{
	struct pt_daemon_proc *proc = handle->data;
	proc->running = false;
}

void tty_close_cb(uv_handle_t *handle)
{
	struct pt_daemon_proc *proc = handle->data;
	uv_close((uv_handle_t*)&proc->proc, pt_daemon_close_handle_cb);
}

void exit_cb(uv_process_t *process, int64_t exit_status, int term_signal)
{
	struct pt_daemon_proc *proc = process->data;
	proc->exit_status = exit_status;
	proc->term_signal = term_signal;

	printf("process_exit:status:%lld   signal:%d  is_work:%d\n",exit_status, term_signal, proc->is_logic);

	uv_close((uv_handle_t*)&proc->tty.pipe, tty_close_cb);
}

void exepath_update()
{
	exepath_size = sizeof(exepath);
	int r = uv_exepath(exepath, &exepath_size);

	if( r != 0 ){
		private_WriteLog(ERROR_LEVEL_FATAL,
				__FUNCTION__,
				__FILE__,
				__LINE__,
				"uv_exepath failed\n");
		exit(1);
	}
}

void spawn_args_reset()
{
	spawn_args_index = 0;
	bzero(&spawn_args, sizeof(spawn_args));
}

void spawn_args_push(char *v)
{
	spawn_args[spawn_args_index++] = v;
}

void spawn_args_done()
{
	spawn_args[0] = "database";
}

char **spawn_args_dup()
{
	int i,len;
	char *copy;
	char **dup_args = calloc(spawn_args_index + 0x1, sizeof(char*));

	for(i = 0; i < spawn_args_index; i++)
	{
		len = strlen(spawn_args[i]) + 0x1;
		copy = malloc(len);

		strcpy(copy,spawn_args[i]);
		dup_args[i] = copy;
	}

	return dup_args;
}

void format_spawn_detach_args()
{
	int i;
	spawn_args_reset();

	for(i = 0; i < daemon_argc; i++)
	{
		spawn_args_push(daemon_argv[i]);
	}
	
	spawn_args_push("run_daemon");
	spawn_args_done();
}

static void tty_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	struct pt_daemon_proc *proc = handle->data;
	struct pt_daemon_tty *tty = &proc->tty;

	assert(suggested_size == sizeof(tty->buff));

	buf->base = tty->buff;
	buf->len = suggested_size;
}


static void tty_print_stderr_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	struct pt_daemon_tty *tty = (struct pt_daemon_tty*)stream;
	static char temp[65537];
	int i,j;
	qboolean found = false;


	if(nread <= 0)
	{
		return;
	}

	memcpy(&tty->strbuf[tty->offset],buf->base,nread);
	tty->offset += nread;

	while(true)
	{
		found = false;
		for( i = 0; i < tty->offset; i ++)
		{
			if(tty->strbuf[i] == '\n')
			{
				i++;

				strncpy(temp, (const char *)&tty->strbuf, i);
				temp[i - 1] = '\0';
				memcpy(tty->strbuf, &tty->strbuf[i], tty->offset - i);
				tty->offset = tty->offset - i;
				found = true;
				break;
			}
		}

		if(found == false) break;

		WriteLog("%s", temp);
		printf("%s\n", temp);
	}
}

struct pt_daemon_proc* spawn_listener()
{
	int i;
	int r;

	struct pt_daemon_proc *proc = calloc(1, sizeof(struct pt_daemon_proc));

	exepath_update();

	proc->options.file = exepath;
	proc->options.flags = 0;
	proc->options.exit_cb = exit_cb;
	proc->proc.data = proc;
	proc->is_logic = false;

	spawn_args_reset();

	for(i = 0; i < daemon_argc; i++)
	{
		if(strcmp(daemon_argv[i],"run_daemon") == 0) continue;
		spawn_args_push(daemon_argv[i]);
	}

	spawn_args_push("--listener");
	spawn_args_done();
	
	proc->options.args = spawn_args_dup();

	proc->tty.offset = 0;

	//stdout / stderr
	uv_pipe_init(uv_default_loop(), &proc->tty.pipe, 0);

	proc->tty.pipe.data = proc;

	proc->options.stdio_count = 3;
	proc->options.stdio = proc->stdio;

	proc->stdio[0].flags = UV_IGNORE;
	proc->stdio[1].flags = UV_IGNORE;
	proc->stdio[2].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;

	proc->stdio[0].data.stream = NULL;
	proc->stdio[1].data.stream = NULL;
	proc->stdio[2].data.stream = (uv_stream_t*)&proc->tty.pipe;

	r = uv_spawn(uv_default_loop(), &proc->proc, &proc->options);

	if( r != 0 ){
		WriteLog("uv_spawn failed:%s\n", uv_strerror(r));
		exit(1);
	}
	
	uv_read_start((uv_stream_t*)&proc->tty.pipe, tty_alloc_cb, tty_print_stderr_cb);
	proc->running = true;

	return proc;
}

struct pt_daemon_proc* spawn_logic()
{
	int i;
	int r;
	struct pt_daemon_proc *proc = calloc(1, sizeof(struct pt_daemon_proc));

	exepath_update();

	proc->options.file = exepath;
	proc->options.flags = 0;
	proc->options.exit_cb = exit_cb;
	proc->proc.data = proc;
	proc->is_logic = true;

	spawn_args_reset();

	for(i = 0; i < daemon_argc; i++)
	{
		if(strcmp(daemon_argv[i],"run_daemon")==0) continue;
		spawn_args_push(daemon_argv[i]);
	}

	spawn_args_push("--logic");
	spawn_args_done();

	proc->options.args = spawn_args_dup();

	proc->tty.offset = 0;

	//stdout / stderr
	uv_pipe_init(uv_default_loop(), &proc->tty.pipe, 0);

	proc->tty.pipe.data = proc;

	proc->options.stdio_count = 3;
	proc->options.stdio = proc->stdio;

	proc->stdio[0].flags = UV_IGNORE;
	proc->stdio[1].flags = UV_IGNORE;
	proc->stdio[2].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;

	proc->stdio[0].data.stream = NULL;
	proc->stdio[1].data.stream = NULL;
	proc->stdio[2].data.stream = (uv_stream_t*)&proc->tty.pipe;

	r = uv_spawn(uv_default_loop(), &proc->proc, &proc->options);

	if( r != 0 ){
		fprintf(stderr, "uv_spawn failed:%s\n", uv_strerror(r));
		exit(1);
	}

	uv_read_start((uv_stream_t*)&proc->tty.pipe, tty_alloc_cb, tty_print_stderr_cb);
	proc->running = true;

	return proc;
}

void spawn_detach()
{
	int r;
	uv_process_options_t options = {0};

	exepath_update();
	format_spawn_detach_args();
	
	options.file = exepath;
	options.args = spawn_args;
	options.flags = UV_PROCESS_DETACHED;
	options.exit_cb = exit_cb;

	r = uv_spawn(uv_default_loop(), &detach_process, &options);

	if( r != 0 ){
		private_WriteLog(ERROR_LEVEL_LOG, __FUNCTION__,
			__FILE__,__LINE__,"uv_spawn failed:%s\n", uv_strerror(r));
		exit(1);
	}
}

void pt_daemon_respawn(struct pt_daemon_proc *proc)
{
	int r;

	WriteLog("Respawn:%d\n", proc->is_logic);

	proc->tty.offset = 0;

	//stdout / stderr
	uv_pipe_init(uv_default_loop(), &proc->tty.pipe, 0);

	proc->tty.pipe.data = proc;

	proc->options.stdio_count = 3;
	proc->options.stdio = proc->stdio;

	proc->stdio[0].flags = UV_IGNORE;
	proc->stdio[1].flags = UV_IGNORE;
	proc->stdio[2].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;

	proc->stdio[0].data.stream = NULL;
	proc->stdio[1].data.stream = NULL;
	proc->stdio[2].data.stream = (uv_stream_t*)&proc->tty.pipe;

	r = uv_spawn(uv_default_loop(), &proc->proc,&proc->options);

	if( r != 0 ){
		WriteLog("Respawn:%d failed\n", proc->is_logic);
		return;
	}
	
	WriteLog("Respawn:%d success\n", proc->is_logic);
	uv_read_start((uv_stream_t*)&proc->tty.pipe, tty_alloc_cb, tty_print_stderr_cb);
	proc->running = true;
}

qboolean is_spawned()
{
	for(int i = 0; i< daemon_argc; i++)
	{
		if(strcmp(daemon_argv[i], "run_daemon") == 0)
		{
			return true;
		}
	}
	return false;
}


void daemon_keep_listener()
{
	if(listener_proc->running == false)
	{
		pt_daemon_respawn(listener_proc);
	}
}

void daemon_keep_logic()
{
	struct pt_daemon_proc *i = logic_procs;

	while(i)
	{
		if(i->running == false)
		{
			pt_daemon_respawn(i);
		}
		i = i->next;
	}
}
void daemon_on_timer(uv_timer_t *timer)
{
	spawn_some_logic_process();
#ifdef USE_LISTENER
	daemon_keep_listener();
#endif

#ifdef USE_LOGIC
	daemon_keep_logic();
#endif
}

static void on_signal_term(uv_signal_t *handle, int signal)
{
	struct pt_daemon_proc *p;

	WriteLog("begin shutdown service.....\n");
	uv_timer_stop(&daemon_timer);
#ifdef USE_LISTENER
	uv_process_kill(&listener_proc->proc, SIGTERM);
#endif

#ifdef USE_LOGIC
	p = logic_procs;

	while(p)
	{
		uv_process_kill(&p->proc, SIGTERM);
		p = p->next;
	}
	
#endif 
	WriteLog("shutdown signal post ok\n");

	sigint_stop();
	uv_signal_stop(&sigterm);
}


void write_pid()
{
	char buf[32];
	int fd;
	size_t pidsize;
	struct flock lock = {0};

	const char *pidfile = PID_FILE;
	fd = open(pidfile, O_CREAT|O_RDWR, 
			S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;

	if (fcntl(fd, F_SETLK, &lock) < 0){
		WriteLog("[DAEMON]service already running...\n");
		exit(0);
	}

	sprintf(buf, "%u",getpid());
	
	pidsize = strlen(buf);

	if (write (fd, buf, pidsize) != pidsize)
	{
		WriteLog("write file failed\n");
		exit(1);
	}
}

static void spawn_some_logic_process()
{
	static qboolean is_spawned_logic = false;
	int i;
	struct pt_daemon_proc *tmp;


	if(is_spawned_logic == true) return;

#ifdef USE_LOGIC
	//spawn some logic process
	for(i = 0; i < process_count; i++)
	{
		tmp = spawn_logic();
		tmp->next = logic_procs;
		logic_procs = tmp;
	}
#endif
	WriteLog("[DAEMON] spawned logic process\n");

	is_spawned_logic = true;
}

//监控进程的入口
int daemon_main(int argc, char *argv[])
{
	daemon_argc = argc;
	daemon_argv = (char**) argv;
	
	if(!is_spawned()){
		WriteLog("[DAEMON] spawn detach terminal process...\n");
		spawn_detach();
		return 0;
	}

	umask(0);
	write_pid();
	
	//捕获信号
	uv_signal_start(&sigterm, on_signal_term, SIGTERM);
	sigint_start();

	WriteLog("[DAEMON] spawned process init....\n");
	//初始化daemon timer
	uv_timer_init(uv_default_loop(), &daemon_timer);
	WriteLog("[DAEMON] uv_timer_init success\n");

	//spawn listener process
#ifdef USE_LISTENER
	listener_proc = spawn_listener();
	WriteLog("[DAEMON] spawned listener process success...\n");
#endif

	uv_timer_start( &daemon_timer, daemon_on_timer, 1000, 1000);
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	return 0;
}
