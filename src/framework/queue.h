#ifndef _QUEUE_INCLUDED_H_
#define _QUEUE_INCLUDED_H_


struct pt_queue
{
	struct pt_queue *prev;
	struct pt_queue *next;
};


void pt_queue_init(struct pt_queue *head);
void pt_queue_push(struct pt_queue *head, struct pt_queue *item);
void pt_queue_remove(struct pt_queue *head,struct pt_queue *item);
void pt_queue_clear(struct pt_queue *head);
struct pt_queue *pt_queue_pop_front(struct pt_queue *head);
qboolean pt_queue_is_empty(struct pt_queue *head);
#endif
