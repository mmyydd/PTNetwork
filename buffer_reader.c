#include <ptnetwork/common.h>
#include <ptnetwork/buffer.h>
#include <ptnetwork/buffer_reader.h>

void buffer_reader_init(struct buffer_reader *reader, struct pt_buffer *buff)
{
    reader->buff = buff;
    reader->pos = 0;
}

void buffer_reader_ignore_bytes(struct buffer_reader *reader, uint32_t n)
{
    reader->pos += n;
}
qboolean buffer_reader_read(struct buffer_reader *reader, void *data, uint32_t length)
{
    if(reader->pos + length > reader->buff->length){
        return false;
    }
    
    memcpy(data, &reader->buff->buff[reader->pos], length);
    reader->pos += length;
    
    return true;
}

qboolean buffer_reader_is_eof(struct buffer_reader *reader)
{
    return reader->pos >= reader->buff->length;
}

unsigned char *buffer_reader_cur_pos(struct buffer_reader *reader)
{
    return &reader->buff->buff[reader->pos];
}
uint32_t buffer_reader_over_size(struct buffer_reader *reader)
{
    return reader->buff->length - reader->pos;
}