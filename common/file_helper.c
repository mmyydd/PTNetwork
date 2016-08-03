//
//  file_helper.c
//  Agent
//
//  Created by yuanrui on 16/7/26.
//  Copyright © 2016年 yuanrui. All rights reserved.
//

#include "file_helper.h"
unsigned char* file_get_buffer(const char *filename, uint32_t *fsize)
{
    unsigned char *buf = NULL;
    long size;
    FILE *fp = fopen(filename, "rb");
    
    if(!fp){
        return NULL;
    }
    
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    
    buf = (unsigned char*)malloc((uint32_t)size);
    fread(buf,size,1,fp);
    fclose(fp);
    
    *fsize = (uint32_t)size;
    
    return buf;
}