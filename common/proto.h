#ifndef _PT_PROTO_H_
#define _PT_PROTO_H_
#include <ptnetwork.h>




/*
 * 内部通信协议
 * */
enum pt_service_identifier_enum
{
	ID_SERVICE_BEGIN = ID_TRANSMIT_JSON + 1,
	
	ID_USER_CONNECTED,
	ID_USER_DISCONNECT,
};


/*
 * 网关服务器回调后端服务器的结构
 * */
struct agent_forward_info
{
	uint64_t user_id;
	unsigned char data[1];
};

#endif
