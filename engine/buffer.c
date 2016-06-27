#include "../common/common.h"
#include "error.h"
#include "buffer.h"


struct pt_buffer* pt_buffer_new(uint32_t length)
{
    struct pt_buffer* buff;
    
    buff = (struct pt_buffer*)malloc(sizeof(struct pt_buffer));
    if(buff == NULL){
        FATAL("malloc",__FUNCTION__, __FILE__, __LINE__);
    }
    buff->length = 0;
    buff->max_length = ALIGN_SIZE(length, PAGESIZE);
    buff->buff = (unsigned char*)malloc(buff->max_length);
    
    if(buff->buff == NULL){
        FATAL("malloc",__FUNCTION__, __FILE__, __LINE__);
    }
    
    buff->need_free = true;
    
    return buff;
};


void pt_buffer_init(struct pt_buffer *buff, uint32_t length)
{
	buff->length = 0;
	buff->max_length = ALIGN_SIZE(length, PAGESIZE);
	buff->buff = (unsigned char*)malloc(buff->max_length);
    if(buff->buff == NULL){
        FATAL("malloc", __FUNCTION__, __FILE__, __LINE__);
    }
	buff->need_free = false;
}

void pt_buffer_free(struct pt_buffer *buff)
{
	free(buff->buff);

	if(buff->need_free == true){
		free(buff);
	}
}

void pt_buffer_reserve(struct pt_buffer *buff, uint32_t length)
{
	uint32_t new_length = buff->length + length;

	new_length = ALIGN_SIZE(new_length, PAGESIZE);

	buff->buff = (unsigned char*)realloc(buff->buff, new_length);
    if(buff->buff == NULL){
        FATAL("realloc", __FUNCTION__, __FILE__, __LINE__);
    }
	buff->max_length = new_length;
}

void pt_buffer_write(struct pt_buffer *buff, unsigned char *data, uint32_t length)
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


qboolean pt_buffer_read(struct pt_buffer *buff,unsigned char **data, uint32_t length, qboolean alloc)
{
	uint32_t copy_length;
	if(length > buff->length) {
		return false;
	}
	
	if(alloc)
	{
		*data = (unsigned char*)malloc(length);
	}
    
    if(*data == NULL){
        FATAL("*data == NULL", __FUNCTION__, __FILE__, __LINE__);
    }

	memcpy(*data, buff->buff, length);

	copy_length = buff->length - length;

	memcpy(buff->buff, &buff->buff[length],copy_length);

	buff->length = copy_length;

	return true;
}
