#include "common.h"
#include "mymemory.h"
#include "error.h"

#define MEMORY_DEBUG_LIMIT 100000

struct memory_block
{
	struct memory_block *next;
	const char *file;
	const char *function;
	int line;
	void *ptr;
};

struct memory_block* memory_block[MEMORY_DEBUG_LIMIT] = {NULL};
qboolean memory_debug_is_enable = false;


unsigned int hash_ptr(void *ptr)
{
#ifdef __x86_64__
	uint64_t hash_id_64;
	hash_id_64 = (uint64_t)ptr % MEMORY_DEBUG_LIMIT;
	return (unsigned int)hash_id_64;
#else
	uint32_t hash_id_32;
	hash_id_32 = (uint32_t)ptr % MEMORY_DEBUG_LIMIT;
	return hash_id_32;
#endif
}

void remove_ptr(void *ptr)
{
	unsigned int index;
	struct memory_block *tmp;
	struct memory_block *prev;
	if(memory_debug_is_enable)
	{
		index = hash_ptr(ptr);

		if(memory_block[index])
		{
			if(memory_block[index]->ptr == ptr)
			{
				tmp = memory_block[index];
				memory_block[index] = tmp->next;
				free(tmp);
			}
			else
			{
				prev = memory_block[index];

				while(prev->next)
				{
					tmp = prev->next;

					if(tmp->ptr == ptr)
					{
						prev->next = tmp->next;
						free(tmp);
						break;
					}

					prev = prev->next;
				}
			}
		}
	}
}

void register_ptr(void *ptr, const char* function, const char *file, int line)
{
	struct memory_block *curr_block;
	unsigned int index;
	if(memory_debug_is_enable)
	{
		curr_block = malloc(sizeof(struct memory_block));
		curr_block->file = file;
		curr_block->line = line;
		curr_block->function = function;
		curr_block->ptr = ptr;
		index = hash_ptr(ptr);


		curr_block->next = memory_block[index];
		memory_block[index] = curr_block;
	}
}
void *emalloc(size_t size, const char *function, const char *file, int line)
{
    void* ptr = malloc(size);
    
    if( ptr == NULL )
    {
        FATAL("allocate memory failed", function,file,line);
    }
	register_ptr(ptr, function, file, line);
    return ptr;
}


void efree(void *ptr, const char *function, const char *file, int line)
{
	remove_ptr(ptr);
    free(ptr);
}

void* erealloc(void* ptr, size_t n, const char *function, const char *file, int line)
{
	void *new_ptr;

	remove_ptr(ptr);
	
    new_ptr = realloc(ptr, n);
    
    if( new_ptr == NULL )
    {
        FATAL("allocate memory failed", function,file,line);
    }

	register_ptr(new_ptr, function, file, line);
    
    return new_ptr;
}


void* ecalloc(size_t count, size_t size, const char *function, const char *file, int line)
{
    void * ptr = calloc(count, size);
    
    if( ptr == NULL )
    {
        FATAL("allocate memory failed", function,file,line);
    }

	register_ptr(ptr, function, file, line);
    
    return ptr;
}

void enable_memory_debugger()
{
	memory_debug_is_enable = true;
}

void dump_leak()
{
	int i;
	struct memory_block *block;
	for(i =0; i < MEMORY_DEBUG_LIMIT; i ++)
	{
		block = memory_block[i];

		while(block)
		{
			fprintf(stderr, "memory leaks: %p  function:%s  file:%s  line:%d\n", block->ptr, block->function, block->file, block->line);
			block = block->next;
		}
	}
}
