#include "common.h"
#include "buffer.h"
#include "pt_error.h"
#include "crc32.h"
#include "packet.h"

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
    uint32_t length;

    hdr = (struct net_header*)netbuf->buff;
	length = hdr->length;
    buf = pt_buffer_new(length);
		
    if(pt_buffer_read(netbuf, buf->buff, length, true) == false){
        pt_buffer_free(buf);
        return NULL;
    }
    
    buf->length = length;
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

struct net_header pt_create_nethdr(uint16_t id)
{
    struct net_header hdr = {0};
    
    hdr.id = id;
    hdr.magic = PACKET_MAGIC;
    
    return hdr;
}

qboolean pt_decrypt_package(uint32_t serial,RC4_KEY *ctx, struct pt_buffer *buff)
{
    struct net_header *hdr = (struct net_header*)buff->buff;
    unsigned char *data = pt_get_packet_buffer(buff);
    uint32_t length = pt_get_packet_size(buff);
    
    if(length < sizeof(uint32_t)){
        PT_TRACE("length < sizeof(uint32_t)", __FUNCTION__, __FILE__, __LINE__);
        return false;
    }
    
    RC4(ctx,length,data,data);
    
    if(*(uint32_t*)data != serial){
        printf("serial:%08x  true:%08x\n",*(uint32_t*)data,serial);
        
        PT_TRACE("data != serial", __FUNCTION__, __FILE__, __LINE__);
        return false;
    }
    
    if(crc32(0, data, length) != hdr->crc){
        PT_TRACE("crc32(0, data, length) != hdr->crc", __FUNCTION__, __FILE__, __LINE__);
        return false;
    }
    
    return true;
}

struct pt_buffer * pt_create_encrypt_package(RC4_KEY *ctx, uint32_t *serial,
                               struct net_header hdr,unsigned char* data, uint32_t length)
{
    struct pt_buffer *buff;
    struct net_header *new_hdr;
    
    
    buff = pt_buffer_new(256);
    pt_buffer_write(buff, (unsigned char*)&hdr, sizeof(struct net_header));
    pt_buffer_write(buff, (unsigned char*)serial, sizeof(uint32_t));
    
	if(data != NULL)
	{
		pt_buffer_write(buff, data, length);
	}

    new_hdr = (struct net_header *)buff->buff;
    unsigned char *encrypt_beg = pt_get_packet_buffer(buff);
    uint32_t encrypt_size = pt_get_packet_size(buff);
    
    new_hdr->crc = crc32(0, encrypt_beg, encrypt_size);
    
    RC4(ctx,encrypt_size,encrypt_beg,encrypt_beg);
    
    new_hdr->length = buff->length;
    *serial = *serial + 1;
    
    return buff;
}

struct pt_buffer *pt_create_package(struct net_header hdr,void* data, uint32_t length)
{
    struct pt_buffer *buff = pt_buffer_new(256);
    struct net_header *new_hdr;
    
    pt_buffer_write(buff, (unsigned char*)&hdr, sizeof(struct net_header));
    
	if(data != NULL) pt_buffer_write(buff, data, length);
    
    new_hdr = (struct net_header *)buff->buff;
    new_hdr->length = buff->length;
    
    return buff;
}
