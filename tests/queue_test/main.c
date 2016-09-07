#include "../../src/framework/common.h"
#include <stdio.h>
#include <queue.h>
#include <string.h>
#include <stdlib.h>


struct myqueue
{
	struct pt_queue eq;
	char msg[100];
};
int serial = 0;

struct myqueue *allocx()
{
	struct myqueue *eq;

	eq = malloc(sizeof(struct myqueue));

	bzero(eq, sizeof(struct myqueue));
	sprintf(eq->msg, "%d", ++serial);

	return eq;
}




int main()
{
	struct pt_queue head;
	int i;
	struct pt_queue *eq;
	struct myqueue *myeq;
	pt_queue_init(&head);


	for(int i =0;i<100;i++)
	{
		pt_queue_push(&head, allocx());
	}

	while(!pt_queue_is_empty(&head))
	{
		eq = pt_queue_pop_front(&head);

		myeq = (struct myqueue*)eq;
		printf("%s\n",  myeq->msg);
	}

	for(int i =0;i<100;i++)
	{
		myeq = allocx();
		pt_queue_push(&head, myeq);
	}


	pt_queue_remove(&head, myeq);




	while(!pt_queue_is_empty(&head))
	{
		eq = pt_queue_pop_front(&head);

		myeq = (struct myqueue*)eq;
		printf("%s\n",  myeq->msg);
	}



	return 0;
}
