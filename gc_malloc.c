#include <ptnetwork/common.h>
#include <ptnetwork/gc_malloc.h>

struct st_gcmalloc_record
{
	struct st_gcmalloc_record *next;
	void *ptr;
	size_t size;
};

struct st_gcmalloc_frame
{
	struct st_gcmalloc_frame *prev;
	struct st_gcmalloc_record *records;
};


struct st_gcmalloc_frame *gc_frame;


void gcmalloc_push_frame(struct st_gcmalloc_frame **your_frame)
{
	struct st_gcmalloc_frame *frame = MEM_MALLOC(sizeof(struct st_gcmalloc_frame));

	if(your_frame == NULL)
	{
		your_frame = &gc_frame;
	}

	frame->prev = *your_frame;
	frame->records = NULL;
	*your_frame = frame;
}

void gcmalloc_pop_frame(struct st_gcmalloc_frame **your_frame)
{
	struct st_gcmalloc_frame *frame;
	struct st_gcmalloc_record *prev, *curr;

	if(your_frame == NULL){
		your_frame = &gc_frame;
	}

	if(*your_frame)
	{
		frame = *your_frame;
		*your_frame = frame->prev;

		prev = NULL;
		curr = frame->records;

		while(curr)
		{
			MEM_FREE(curr->ptr);
			
			prev = curr;
			curr = curr->next;
			MEM_FREE(prev);
		}

		MEM_FREE(frame);
	}
}

void* gcmalloc_alloc(struct st_gcmalloc_frame **your_frame, size_t n)
{
	struct st_gcmalloc_record *record;

	if(your_frame == NULL){
		your_frame = &gc_frame;
	}

	assert((*your_frame) != NULL);

	record = MEM_MALLOC(sizeof(struct st_gcmalloc_record));
	record->size = n;
	record->ptr = MEM_MALLOC(n);
	record->next = (*your_frame)->records;
	(*your_frame)->records = record;

	return record->ptr;
}
