#ifndef _PT_PROTO_INCLUED_H_
#define _PT_PROTO_INCLUED_H_

#pragma pack(1)
#define PACKET_MAGIC 'ZZDM'

/*
    chacha20算法支持库：rc4
 */

struct net_header
{
    //包头 等于PACKET_MAGIC
	uint32_t magic;
    //包长度，需要计算sizeof(struct net_header)的大小
	uint32_t length;
    //包类型ID
	uint16_t id;
    //哈希数据
    uint32_t crc;
};

/*
 * 注：如果服务器不启用加密 则忽略本段
 *
 * 当服务器启动了加密的时候
 * 强制要求crc值为正确的校验值
 * 强制追加uint32_t serial的数据到包内容
 * crc ＝ serial + data 的数据校验
 * */


/*
 =========================================================================
    所有加密的数据的算法都定为RC4
    加密数据包格式
	struct net_header head;		包头
    uint32_t   serial;          包序列
    unsigned char data[n];      追加的真实数据


	非加密数据格式
	struct net_header header;	//包头
	unsigned char data[n];		//包数据
 =========================================================================
 */

enum protocol_enum_id
{
	//心跳包，检查远端连接是否有效。
	ID_TRANSMIT_KEEPALIVE = 1,

	//传送JSON值到另外一端。
	ID_TRANSMIT_JSON,

	//通知服务器用户连接断开的封包
	ID_USER_CONNECTED,
	ID_USER_DISCONNECTED,
	ID_SENDALL,

	//内部控制包
	ID_CONTROL_PACKET = 100,

    //其他预留封包
    ID_USER_PACKET_ENUM = 500,
};

/*
 =========================================================================
 当数据传输为ID_TRANSMIT_JSON时的JSON结构信息为
 
 {
    "type" : "chat",
    "action": "xxxMethod",
    "params": {
        .....
    }
 }

 =========================================================================
 */

#pragma pack()
#endif
