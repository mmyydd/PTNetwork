#ifndef _PT_COMMON_DEFS_H_
#define _PT_COMMON_DEFS_H_

#include <stdint.h>

typedef int qboolean;

#define true 1
#define false 0
#define TRUE 1
#define FALSE 0


typedef unsigned char byte;


struct pt_wreq
{
    uv_write_t req;
    uv_buf_t buf;
    struct pt_buffer *buff;
    
    void* data;
};

#endif
