//
//  file_helper.h
//  Agent
//
//  Created by yuanrui on 16/7/26.
//  Copyright © 2016年 yuanrui. All rights reserved.
//

#ifndef file_helper_h
#define file_helper_h

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


unsigned char* file_get_buffer(const char *filename, uint32_t *fsize);

#endif /* file_helper_h */
