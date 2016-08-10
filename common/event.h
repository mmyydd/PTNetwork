#ifndef _EVENT_INCLUDED_H_
#define _EVENT_INCLUDED_H_


typedef void (*pt_event_cb)(void *ptr);

struct pt_events
{
	struct pt_events *next;
	int event_id;
	pt_event_cb cb;
};


void pt_event_register(int event_id, pt_event_cb cb);
void pt_event_dispatch(int event_id, void *arg);
void pt_event_unregister(int event_id, pt_event_cb cb);
#endif
