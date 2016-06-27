#ifndef _PT_ENGINE_BUFFER_H_
#define _PT_ENGINE_BUFFER_H_


#define PAGESIZE 0x1000


#define ALIGN_SIZE(n,a) n % a == 0 ? n : ((n / a)+1) * a

struct pt_buffer
{
	unsigned char *buff;
	uint32_t length;
	uint32_t max_length;
	qboolean need_free;
};


struct pt_buffer* pt_buffer_new(uint32_t length);
void pt_buffer_init(struct pt_buffer *buff, uint32_t length);
void pt_buffer_free(struct pt_buffer *buff);
void pt_buffer_reserve(struct pt_buffer *buff, uint32_t length);
void pt_buffer_write(struct pt_buffer *buff, unsigned char *data, uint32_t length);
qboolean pt_buffer_read(struct pt_buffer *buff,unsigned char **data, uint32_t length, qboolean alloc);

#endif
