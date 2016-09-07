#ifndef _FRIEND_INCLUDED_H_
#define _FRIEND_INCLUDED_H_

#include <string>
#include <stdint.h>
#include <vector>

//聊天包信息
struct pt_message_pack
{
	uint32_t from_id;

	uint32_t to_id;

	std::string content;
};

enum pt_player_state
{
	//玩家离线
	PLAYER_STATE_OFFLINE = -1,
	//在大厅
	PLAYER_STATE_LOBBY = 0,
	//在房间中
	PLAYER_STATE_IN_LOBBY = 1,
	//队列中
	PLAYER_STATE_QUEUE = 2,
	//正在游戏中
	PLAYER_STATE_PLAYING = 3
};

struct pt_player
{
	uint32_t user_id;
	//玩家的状态
	enum pt_player_state state;

	//名字和头像
	std::string name;
	std::string avatar;

	bool is_vip;

	//战队
	uint32_t clan_id;
	std::string clan_name;

	pt_player()
	{
		state = PLAYER_STATE_OFFLINE;
		user_id = 0;
		clan_id = 0;
		is_vip = false;
	}
};

struct pt_player_friend_info
{
	uint32_t user_id;
	bool is_black;
};

typedef std::vector<pt_player_friend_info> pt_player_friends;

class friend_service
{
public:
	//加载玩家信息到程序中
	void loadPlayer(uint32_t user_id);
	
	//添加好友
	void addFriend();

	//删除好友
	void removeFriend();
	
	//修改好友
	void updateFriend();

	//添加好友申请结果
	void friendAccept();

	//获取好友列表
	void friendList();

	//发送私聊信息
	void sendMessage();
};


#endif
