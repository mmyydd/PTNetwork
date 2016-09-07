#ifndef _LOBBY_INCLUDED_H_
#define _LOBBY_INCLUDED_H_
#include <string>
#include <set>
#include <map>
#include <json/json.h>
enum pt_gamemode
{
	GAMEMODE_NORMAL,
	GAMEMODE_PVP
};


class pt_player
{
public:
	uint32_t user_id;
	int team;
	std::string name;
};

class pt_matchOption
{
public:
	bool is_verify;
	int area;
	std::set<std::string> m_matchMaps;

	std::string useMap;
	Json::Value gameDesc;
};
class pt_lobby
{
public:
	pt_gamemode m_gamemode;

	//大厅所有者id
	uint32_t owner_id;
	//大厅名称
	std::string name;
	std::string password;

	std::vector<pt_player> m_players;
};


class pt_lobby_list
{
	void createLobby();
	void enterLobby();
	void leaveLobby();
	void inviteBuddyToLobby();
	void queryLobby();
	void updateLobby();
	void listLobby();

	void startMatch();
	void cancelMatch();
};

#endif
