#ifndef _PT_DEFTYPE_INCLUED_H_
#define _PT_DEFTYPE_INCLUED_H_

typedef int qboolean;

#define true 1
#define false 0
#define TRUE 1
#define FALSE 0


typedef unsigned char byte;


union pt_net_s {
    uv_stream_t stream;
    uv_tcp_t tcp;
    uv_pipe_t pipe;
};

typedef union pt_net_s pt_net_t;


struct pt_wreq
{
    uv_write_t req;
    uv_buf_t buf;
    struct pt_buffer *buff;
    
    void* data;
};

#endif
