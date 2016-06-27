#ifndef _COMMON_PROTOCOL_H_
#define _COMMON_PROTOCOL_H_

#pragma pack(1)
#define PACKET_MAGIC 0xABCD

/*
    chacha20算法支持库：libsodium
 */

struct net_header
{
    //包头 等于PACKET_MAGIC
	uint16_t magic;
    //包长度，需要计算sizeof(struct net_header)的大小
	uint32_t length;
    //包类型ID
	uint16_t id;
    //回调到特殊节点服务器，如大厅服务器，聊天服务器，匹配服务器等
    uint8_t forward_to;
    //包内容是否加密，如果数据包已加密
    uint8_t encrypt;
    //哈希数据
    uint32_t crc;
};

/*
 =========================================================================
    所有加密的数据的算法都定为chacha20
    加密数据包格式
    uint32_t   timestamp;       对于需要加密的数据，使用timestamp防止重放攻击
    unsigned char data[n];      追加的真实数据
 =========================================================================
 */

enum protocol_enum_id
{
	//心跳包，检查远端连接是否有效。
	ID_TRANSMIT_KEEPALIVE = 1,

	//传送JSON值到另外一端。
	ID_TRANSMIT_JSON,

    //内网服务器交互封包
	ID_RESERVE_TRANSMIT_ENUM = 10000,

    //客户端请求包
    ID_USER_CLIENT_ENUM = 20000,
    
	//游戏服务器请求包
	ID_USER_SERVER_ENUM = 30000,
    
    //其他预留封包
    ID_USER_PACKET_ENUM = 40000,
};

/*
 =========================================================================
 当数据传输为ID_TRANSMIT_JSON时  强制使用chacha20算法进行加密
 并且设置encrypt = true且计算原始json的crc值
 
 {
    "action": "xxxMethod",
    "params": {
        .....
    }
 }

 =========================================================================
 */

#pragma pack()
#endif
