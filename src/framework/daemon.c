#include "common.h"
#include "daemon.h"



struct pt_daemon;
struct pt_subprocess
{
	struct pt_subprocess *next;
	struct pt_daemon *daemon;
	uv_process_t process;
	
	int number_of_push;
	int number_of_pop;
	int number_of_reboot;
	uint64_t update_time;
};


struct pt_daemon
{
	struct pt_subprocess *process;
	uv_pipe_t channel;
	int last_error;
};



struct pt_subprocess* pt_subprocess_new()
{
	struct pt_subprocess *process;

	process = MEM_MALLOC(sizeof(struct pt_subprocess));
	bzero(process, sizeof(struct pt_subprocess));

	process->update_time = uv_hrtime();

	return process;
}


void pt_subprocess_free(struct pt_subprocess *process)
{
	MEM_FREE(process);
}

struct pt_daemon* pt_daemon_new()
{
	struct pt_daemon *daemon;

	daemon = MEM_MALLOC(sizeof(struct pt_daemon));

	bzero(daemon, sizeof(struct pt_daemon));

	return daemon;
}


void pt_daemon_free(struct pt_daemon *daemon)
{
	MEM_FREE(daemon);
}

qboolean pt_daemon_init(struct pt_daemon *daemon, const char *pipe)
{
	daemon->last_error = uv_pipe_init(uv_default_loop(), &daemon->channel, 1);
	if(daemon->last_error != 0){
		return false;
	}
	
	daemon->last_error = uv_pipe_bind(&daemon->channel, pipe);

	if(daemon->last_error != 0){
		return false;
	}

	return true;
}
