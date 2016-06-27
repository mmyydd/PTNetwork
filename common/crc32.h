//
//  crc32.h
//  xcode
//
//  Created by 袁睿 on 16/6/23.
//  Copyright © 2016年 number201724. All rights reserved.
//

#ifndef crc32_h
#define crc32_h

#include <stdio.h>

uint32_t
crc32(uint32_t crc, const void *buf, size_t size);
#endif /* crc32_h */
