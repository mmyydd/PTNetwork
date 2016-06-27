#ifndef _COMMON_PACKET_H_
#define _COMMON_PACKET_H_

#include "../common/proto.h"



#define PACKET_INFO_OK 0
#define PACKET_INFO_OVERFLOW 1
#define PACKET_INFO_SMALL 2
#define PACKET_INFO_FAKE 3

//每个包的最大大小，超过此大小则认为非法包
extern uint32_t pt_max_pack_size;

qboolean pt_get_packet_status(struct pt_buffer *buf, uint32_t *err);

struct pt_buffer* pt_split_packet(struct pt_buffer *netbuf);

unsigned char *pt_get_packet_buffer(struct pt_buffer *netbuf);
uint32_t pt_get_packet_size(struct pt_buffer *netbuf);

void pt_create_nethdr(struct net_header *hdr, uint16_t id, uint32_t buf_len);
struct pt_buffer* pt_create_buffer(struct net_header *hdr, unsigned char *buf, uint32_t buf_len);


//验证一个包的CRC值是否正确
qboolean pt_verify_package_crc(struct pt_buffer *buff);

//加密/解密包数据
qboolean pt_encrypt_package(struct pt_buffer *buff);
#endif
