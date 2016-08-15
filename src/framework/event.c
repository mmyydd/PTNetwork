#include "common.h"
#include "event.h"


static struct pt_events *g_events = NULL;


void pt_event_register(int event_id, void* data, pt_event_cb cb)
{
	struct pt_events *ev = MEM_MALLOC(sizeof(struct pt_events));
	bzero(ev, sizeof(struct pt_events));
	
	ev->event_id = event_id;
	ev->cb = cb;
	ev->next = g_events;
	ev->data = data;
	g_events = ev;
}


void pt_event_dispatch(int event_id, void *arg)
{
	struct pt_events *ev = g_events;

	while(ev)
	{
		if(ev->event_id == event_id)
		{
			ev->cb(arg);
		}
		ev = ev->next;
	}
}



void pt_event_unregister(int event_id, void *data, pt_event_cb cb)
{
	struct pt_events *prev, *next;

	prev = NULL;
	next = g_events;

	while(next)
	{
		if(next->cb == cb && next->event_id == event_id && next->data == data)
		{
			if(prev)
			{
				prev->next = next->next;
				MEM_FREE(next);
				next = prev->next;
				continue;
			}
			else
			{
				prev = next;
				next = next->next;
				MEM_FREE(prev);
				prev = NULL;
				continue;
			}
		}
		prev = next;
		next = next->next;
	}
}
