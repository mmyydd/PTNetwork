#include "common.h"
#include "error.h"
#include "buffer.h"

struct pt_buffer_allocator buffer_allocator = {0, 10000, false, NULL};

static struct pt_buffer* pt_buffer_create(uint32_t length)
{
    struct pt_buffer* buff;
    
    buff = (struct pt_buffer*)malloc(sizeof(struct pt_buffer));
    if(buff == NULL){
        FATAL("pt_buffer_new malloc",__FUNCTION__, __FILE__, __LINE__);
        abort();
    }
    
    buff->next = NULL;
    buff->length = 0;
    buff->max_length = ALIGN_SIZE(length, PAGESIZE);
    buff->buff = (unsigned char*)malloc(buff->max_length);
    
    if(buff->buff == NULL){
        FATAL("pt_buffer_new malloc",__FUNCTION__, __FILE__, __LINE__);
        abort();
    }
    
    return buff;
};

static void pt_buffer_release(struct pt_buffer *buff)
{
    assert(buff != NULL);
    
    free(buff->buff);
    free(buff);
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
    if(buffer_allocator.enable == false){
        return pt_buffer_create(length);
    }
    
    struct pt_buffer* buff = pt_buffer_alloc_by_allocator();
    
    if(buff == NULL){
        return pt_buffer_create(length);
    }
    
    pt_buffer_reserve(buff, length);
    return buff;
};

void pt_buffer_free(struct pt_buffer *buff)
{
    
    if(buffer_allocator.enable == false){
        pt_buffer_release(buff);
        return;
    }
    
    pt_buffer_free_by_allocator(buff);
}

void pt_buffer_reserve(struct pt_buffer *buff, uint32_t length)
{
    uint32_t new_length = buff->length + length;
    
    if(new_length < buff->max_length) return;
    
    new_length = ALIGN_SIZE(new_length, PAGESIZE);
    
    buff->buff = (unsigned char*)realloc(buff->buff, new_length);
    if(buff->buff == NULL){
        FATAL("pt_buffer_reserve realloc", __FUNCTION__, __FILE__, __LINE__);
        abort();
    }
    buff->max_length = new_length;
}

void pt_buffer_write(struct pt_buffer *buff, const unsigned char *data, uint32_t length)
{
    if(buff->length + length < buff->max_length){
        memcpy(&buff->buff[buff->length],data,length);
        buff->length += length;
    } else {
        pt_buffer_reserve(buff, length);
        memcpy(&buff->buff[buff->length],data,length);
        buff->length += length;
    }
}

qboolean pt_buffer_read(struct pt_buffer *buff,unsigned char *data, uint32_t length, qboolean remove)
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