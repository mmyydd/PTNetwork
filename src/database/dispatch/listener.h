#ifndef _LISTENER_INCLUDED_H_
#define _LISTENER_INCLUDED_H_

int listener_main(int argc, char *argv[]);

struct worker_object
{
	struct worker_object *next;
	struct pt_sclient *user;
	time_t tm;
	qboolean is_busy;
};


#endif
