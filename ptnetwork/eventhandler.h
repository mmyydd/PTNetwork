#ifndef _PT_EVENTHANDLER_H_INCLUDED_
#define _PT_EVENTHANDLER_H_INCLUDED_

struct pt_event_handler;

typedef void (*pt_event_notification)(struct pt_event_handler *handler, void *userdata);

struct pt_event_handler
{
	struct pt_event_handler *next;
	uint64_t event_id;
	pt_event_notification cb;
	void *data;
};


struct pt_event_listener
{
	struct pt_table *event_table;
};

void pt_event_init();
void pt_event_shutdown();

void pt_event_register(struct pt_event_handler *handler);
void pt_event_dispatch(uint64_t event_id, void *userdata);
#endif
