#include "common.h"
#include "queue.h"


void pt_queue_init(struct pt_queue *head)
{
	head->next = head;
	head->prev = head;
}


void pt_queue_push(struct pt_queue *head, struct pt_queue *item)
{
	item->prev = head->prev;
	item->next = head;
	head->prev = item;
	item->prev->next = item;
}

void pt_queue_remove(struct pt_queue *head,struct pt_queue *item)
{
	struct pt_queue *iter = head->next;

	while(iter != head)
	{
		if(iter == item)
		{
			iter->prev->next = iter->next;
			iter->next->prev = iter->prev;
		}
		iter = iter->next;
	}
}


struct pt_queue *pt_queue_pop_front(struct pt_queue *head)
{
	struct pt_queue *node;
	if(head->next == head){
		return NULL;
	}

	node = head->next;
	
	head->next = node->next;
	node->next->prev = head;

	return node;
}

qboolean pt_queue_is_empty(struct pt_queue *head)
{
	return head->next == head && head->prev == head;
}

void pt_queue_clear(struct pt_queue *head)
{
	head->next = head->prev = head;
}
