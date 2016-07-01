//
//  buffer_reader.h
//  JsonEcho
//
//  Created by 袁睿 on 2016/6/29.
//  Copyright © 2016年 number201724. All rights reserved.
//

#ifndef buffer_reader_h
#define buffer_reader_h

struct buffer_reader
{
    struct pt_buffer *buff;
    uint32_t pos;
};


void buffer_reader_init(struct buffer_reader *reader, struct pt_buffer *buff);
void buffer_reader_ignore_bytes(struct buffer_reader *reader, uint32_t n);
qboolean buffer_reader_read(struct buffer_reader *reader, unsigned char *data, uint32_t length);
qboolean buffer_reader_is_eof(struct buffer_reader *reader);
unsigned char *buffer_reader_cur_pos(struct buffer_reader *reader);
uint32_t buffer_reader_over_size(struct buffer_reader *reader);


#endif /* buffer_reader_h */
