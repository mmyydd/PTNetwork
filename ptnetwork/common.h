#ifndef _PT_COMMON_INCLUED_H_
#define _PT_COMMON_INCLUED_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <uv.h>
#include <assert.h>
#include <openssl/rc4.h>

#include <ptnetwork/def_types.h>
#include <ptnetwork/mymemory.h>


//windows mingw32 supports
#ifdef _WIN32
#undef ERROR
#undef MEM_FREE
#endif

#define USER_DEFAULT_BUFF_SIZE 512

#endif
