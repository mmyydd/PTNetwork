//
//  crc32.h
//  xcode
//
//  Created by 袁睿 on 16/6/23.
//  Copyright © 2016年 number201724. All rights reserved.
//

#ifndef _PT_CRC32_INCLUED_H_
#define _PT_CRC32_INCLUED_H_

#include <stdio.h>
#include <stdint.h>

uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

#endif
