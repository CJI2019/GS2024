#pragma once

enum class PlayerMoveDir : BYTE {
	LEFT = 0,
	RIGHT,
	UP,
	DOWN
};

struct PlayerInfo {
	POINT pos;
};

class ClientInfo
{
public:
	ClientInfo();
	ClientInfo(SOCKET sock, int id);
	~ClientInfo();
private:
	SOCKET sock{};
	int id{};

public:
	SOCKET GetSock();
	int GetId();

	void Exit();


	PlayerInfo playerinfo;
	void PlayerMove(PlayerMoveDir dir);
	void WriteData(vector<BYTE>& buffer);
};

