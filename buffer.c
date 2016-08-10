#include <ptnetwork/common.h>
#include <ptnetwork/error.h>
#include <ptnetwork/buffer.h>

struct pt_buffer_allocator buffer_allocator = {0, 10000, false, NULL};

uint64_t number_of_borrow = 0;
uint64_t number_of_back = 0;
uint64_t number_of_use_buffer = 0;


uint64_t pt_buffer_get_borrow_count()
{
	return number_of_borrow;
}
uint64_t pt_buffer_get_back_count()
{
	return number_of_back;
}
uint64_t pt_buffer_get_use_count()
{
	return number_of_use_buffer;
}

uint32_t pt_buffer_ref_increment(struct pt_buffer *buff)
{
    return ++buff->ref_count;
}

uint32_t pt_buffer_ref_decrement(struct pt_buffer *buff)
{
    return --buff->ref_count;
}

static struct pt_buffer* pt_buffer_create(uint32_t length)
{
    struct pt_buffer* buff;
    
    buff = (struct pt_buffer*)MEM_MALLOC(sizeof(struct pt_buffer));
    
    buff->next = NULL;
    buff->length = 0;
    buff->max_length = ALIGN_SIZE(length, PAGESIZE);
    buff->buff = (unsigned char*)MEM_MALLOC(buff->max_length);
    buff->ref_count = 0;
    
    return buff;
};

static void pt_buffer_release(struct pt_buffer *buff)
{
    assert(buff != NULL);

    MEM_FREE(buff->buff);
    MEM_FREE(buff);
}

static struct pt_buffer *pt_buffer_alloc_by_allocator()
{
    struct pt_buffer *buff;
    
    if(buffer_allocator.buff == NULL)
        return NULL;
    
    buffer_allocator.buffer_count--;
    buff = buffer_allocator.buff;
    buffer_allocator.buff = buff->next;
    buff->next = NULL;
    buff->length = 0;
    buff->ref_count = 0;
    
    return buff;
}

static void pt_buffer_free_by_allocator(struct pt_buffer *buff)
{
    if(buffer_allocator.buffer_count >= buffer_allocator.buffer_max_count){
        pt_buffer_release(buff);
        return;
    }
    
    buff->next = buffer_allocator.buff;
    buffer_allocator.buff = buff;
    buffer_allocator.buffer_count ++;
}


void pt_buffer_set_allocator_count(uint32_t count)
{
    buffer_allocator.buffer_max_count = count;
}

void pt_buffer_enable_allocator(qboolean enable)
{
    buffer_allocator.enable = enable;
}

void pt_buffer_clear_allocator()
{
    struct pt_buffer *curr;
    while(buffer_allocator.buff)
    {
        curr = buffer_allocator.buff;
        buffer_allocator.buff = curr->next;
        
        pt_buffer_release(curr);
    }
    
    buffer_allocator.buffer_count = 0;
}

struct pt_buffer* pt_buffer_new(uint32_t length)
{
    struct pt_buffer* buff = NULL;

	if(length == 0)
	{
		length = PAGESIZE;
	}

    if(buffer_allocator.enable == false)
    {
        buff = pt_buffer_create(length);
    }
    else 
    {
        buff = pt_buffer_alloc_by_allocator();
        if(buff == NULL){
            buff = pt_buffer_create(length);
        }
    }

    assert(buff != NULL);
    
    pt_buffer_reserve(buff, length);
    pt_buffer_ref_increment(buff);
	number_of_use_buffer++;
	number_of_borrow++;

    return buff;
};

void pt_buffer_free(struct pt_buffer *buff)
{
    assert(buff != NULL);
    
    uint32_t ref_count = pt_buffer_ref_decrement(buff);
    
    assert(ref_count >= 0);

    if(ref_count == 0)
    {
		number_of_use_buffer--;
		number_of_back++;
        if(buffer_allocator.enable == false)
        {
            pt_buffer_release(buff);
        }
        else
        {
            pt_buffer_free_by_allocator(buff);
        }
    }

}

void pt_buffer_reserve(struct pt_buffer *buff, uint32_t length)
{
    uint32_t new_length = buff->length + length;
    
    if(new_length < buff->max_length) return;
    
    new_length = ALIGN_SIZE(new_length, PAGESIZE);
    
    buff->buff = (unsigned char*)MEM_REALLOC(buff->buff, new_length);
    buff->max_length = new_length;
}

void pt_buffer_write(struct pt_buffer *buff, const void *data, uint32_t length)
{
    uint32_t over_size = buff->length + length;
    
    if(over_size <= buff->max_length){
        memcpy(&buff->buff[buff->length],data,length);
        buff->length += length;
    } else {
        pt_buffer_reserve(buff, length);
        
        memcpy(&buff->buff[buff->length],data,length);
        buff->length += length;
    }
}

qboolean pt_buffer_read(struct pt_buffer *buff,void *data, uint32_t length, qboolean remove)
{
    uint32_t copy_length;
    
    if(length > buff->length) {
        return false;
    }
    
    memcpy(data, buff->buff, length);
    
    
    if(remove == true)
    {
        copy_length = buff->length - length;
        
        memcpy(buff->buff, &buff->buff[length],copy_length);
        
        buff->length = copy_length;
    }
    
    return true;
}

void DUMP(struct pt_buffer*buff)
{
    uint32_t i = 0;
    
    for(i = 0; i < buff->length; i ++)
    {
        printf("%02X",buff->buff[i]);
    }
    
    printf("\n");
}
