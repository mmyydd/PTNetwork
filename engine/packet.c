#include "../common/common.h"
#include "buffer.h"
#include "packet.h"
#include "error.h"
#include <sodium.h>
#include "crc32.h"


static uint32_t chacha20_key[4] = {0x42970C86,0xA0B3A057,0x51B97B3C,0x70F8891E};
static uint64_t chacha20_nonce = 0x4146C0A2868B73A9;

uint32_t pt_max_pack_size = 0x10000;


qboolean pt_get_packet_status(struct pt_buffer *buf, uint32_t *err)
{
	struct net_header *hdr;
    
	if(buf->length < sizeof(struct net_header)){
		*err = PACKET_INFO_SMALL;
		return false;
	}
	
	hdr = (struct net_header*)buf->buff;
	
	if(hdr->magic != PACKET_MAGIC){
		*err = PACKET_INFO_FAKE;
		return false;
	}
    
    if(hdr->length > pt_max_pack_size){
        *err = PACKET_INFO_OVERFLOW;
        return false;
    }
    
    if(hdr->length > buf->length){
        *err = PACKET_INFO_SMALL;
        return false;
    }

	*err = PACKET_INFO_OK;
	return true;
}

struct pt_buffer* pt_split_packet(struct pt_buffer *netbuf)
{
    struct pt_buffer *buf;
    struct net_header *hdr;
    
    hdr = (struct net_header*)netbuf->buff;
    buf = pt_buffer_new(hdr->length);
    if(buf == NULL){
        FATAL("pt_buffer_new == NULL", __FUNCTION__, __FILE__, __LINE__);
    }
    pt_buffer_read(netbuf, &buf->buff, hdr->length, false);
    buf->length = hdr->length;
    return buf;
}

unsigned char *pt_get_packet_buffer(struct pt_buffer *netbuf)
{
    return &netbuf->buff[sizeof(struct net_header)];
}

uint32_t pt_get_packet_size(struct pt_buffer *netbuf)
{
    return netbuf->length - sizeof(struct net_header);
}

void pt_create_nethdr(struct net_header *hdr, uint16_t id, uint32_t buf_len)
{
    hdr->length = buf_len + sizeof(struct net_header);
    hdr->id = id;
    hdr->magic = PACKET_MAGIC;
    hdr->encrypt = false;
    hdr->crc = 0;
    hdr->forward_to = 0;
}

struct pt_buffer* pt_create_buffer(struct net_header *hdr, unsigned char *buf, uint32_t buf_len)
{
    struct pt_buffer *buff = pt_buffer_new(hdr->length);
    
    pt_buffer_write(buff, (unsigned char*)hdr, sizeof(struct net_header));
    pt_buffer_write(buff, buf, buf_len);
    
    return buff;
}

qboolean pt_verify_package_crc(struct pt_buffer *buff)
{
    struct net_header *hdr = (struct net_header*)buff->buff;
    unsigned char *buf = pt_get_packet_buffer(buff);
    uint32_t len = pt_get_packet_size(buff);
    uint32_t crc = crc32(0, buf, len);
    
    return crc == hdr->crc;
}


qboolean pt_encrypt_package(struct pt_buffer *buff)
{

    unsigned char *buf = pt_get_packet_buffer(buff);
    uint32_t len = pt_get_packet_size(buff);
    
    //检查timestamp大小
    if(len < sizeof(uint32_t)){
        return false;
    }
    
    crypto_stream_chacha20(buf,len,(const unsigned char*)&chacha20_key, (const unsigned char*)&chacha20_nonce);
    
    return true;
}
