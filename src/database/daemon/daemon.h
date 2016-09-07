#ifndef _DAEMON_INCLUDED_H_
#define _DAEMON_INCLUDED_H_

struct pt_daemon_tty
{
	uv_pipe_t pipe;
	char buff[65536];
	char strbuf[65536];
	uint32_t offset;
};
struct pt_daemon_proc
{
	struct pt_daemon_proc *next;

	qboolean is_logic;
	uv_process_t proc;
	uv_process_options_t options;
	uv_stdio_container_t stdio[3];
	struct pt_daemon_tty tty;
	int64_t exit_status;
	int term_signal;
	qboolean running;
};
int daemon_main(int argc, char *argv[]);

#endif
