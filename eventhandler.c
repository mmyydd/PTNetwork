#include "common.h"
#include "table.h"
#include "eventhandler.h"


struct pt_event_listener g_event_listener;
qboolean g_event_initialized = false;


void pt_event_init()
{
	if(g_event_initialized) return;

	g_event_listener.event_table = pt_table_new_ex(1024);

	g_event_initialized = true;
}

void pt_event_shutdown()
{
	if(g_event_initialized)
	{
		g_event_initialized = false;

		pt_table_free(g_event_listener.event_table);
	}
}

void pt_event_register(struct pt_event_handler *handler)
{
	struct pt_event_handler *prev = pt_table_find(g_event_listener.event_table, handler->event_id);

	if( prev )
	{
		handler->next = prev->next;
		prev->next = handler;
	}
	else
	{
		handler->next = NULL;
		pt_table_insert(g_event_listener.event_table, handler->event_id, handler);
	}
}

void pt_event_dispatch(uint64_t event_id, void *userdata)
{
	struct pt_event_handler *handler = pt_table_find(g_event_listener.event_table, event_id);
	
	while( handler )
	{
		handler->cb(handler, userdata);
		handler = handler->next;
	}
}
