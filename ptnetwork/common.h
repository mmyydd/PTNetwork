#ifndef _PT_COMMON_INCLUED_H_
#define _PT_COMMON_INCLUED_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <uv.h>
#include <assert.h>
#include <openssl/rc4.h>

#include "def_types.h"
#include "mymemory.h"


#define USER_DEFAULT_BUFF_SIZE 512



enum pt_state
{
	//刚初始化后的状态
	PT_STATE_NORMAL,

	//已经初始化过
	PT_STATE_INIT,

	//服务器已经启动
	PT_STATE_START,
	//服务器正在启动中
	PT_STATE_BUSY,
	//服务器已经停止
	PT_STATE_STOP,

	//客户端正在连接服务器中
	PT_STATE_CONNECTING,
	//客户端连接成功
	PT_STATE_CONNECTED,
	//客户端连接服务器失败
	PT_STATE_CONNECT_FAILED
};


enum pt_disconnect_type
{
	DISCONNECT_TYPE_EOF,
	DISCONNECT_TYPE_FAKE,
	DISCONNECT_TYPE_USER,
	DISCONNECT_TYPE_OVERFLOW,
	DISCONNECT_TYPE_CLOSE,
	DISCONNECT_TYPE_SEND_FLOW,
	DISCONNECT_TYPE_READ_FAIL,
	DISCONNECT_TYPE_MAX_CONN,
	DISCONNECT_TYPE_DECRYPT
};


#ifdef _WIN32
void bzero(void *p, int s);
#endif
#endif
