#ifndef _PT_PACKET_INCLUED_H_
#define _PT_PACKET_INCLUED_H_

#include <ptnetwork/proto.h>



#define PACKET_INFO_OK 0
#define PACKET_INFO_OVERFLOW 1
#define PACKET_INFO_SMALL 2
#define PACKET_INFO_FAKE 3

//每个包的最大大小，超过此大小则认为非法包
extern uint32_t pt_max_pack_size;

/*
    获取数据包块的状态，是否完整，是否错误，是否溢出等
    返回true则直接处理完整的数据包
 */
qboolean pt_get_packet_status(struct pt_buffer *buf, uint32_t *err);

/*
    拆分一个数据包,返回新生成的分块数据
    结果需要使用pt_buffer_free来释放
 */
struct pt_buffer* pt_split_packet(struct pt_buffer *netbuf);

/*
    获取pt_buffer的数据指针，而不包括net_header
 */
unsigned char *pt_get_packet_buffer(struct pt_buffer *netbuf);
/*
    获取pt_buffer的数据大小，排除net_header的大小
 */
uint32_t pt_get_packet_size(struct pt_buffer *netbuf);

/*
    为数据包创建一个net_header结构，用于发送到目标端
 */
struct net_header pt_create_nethdr(uint16_t id);


qboolean pt_decrypt_package(uint32_t serial,RC4_KEY *ctx, struct pt_buffer *buff);


struct pt_buffer *pt_create_encrypt_package(RC4_KEY *ctx, uint32_t *serial,
                               struct net_header hdr,unsigned char* data, uint32_t length);


struct pt_buffer *pt_create_package(struct net_header hdr,
                       void* data, uint32_t length);

#endif
