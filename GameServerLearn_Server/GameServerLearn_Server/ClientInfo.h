#pragma once

class EXP_OVER {
public:
	WSAOVERLAPPED wsaover;
	WSABUF	wsabuf[1];
	char	buf[BUFSIZE];
public:
	EXP_OVER(char* data, int m_size)
	{
		ZeroMemory(&wsaover, sizeof(wsaover));
		wsabuf[0].buf = buf;
		wsabuf[0].len = m_size;
		memcpy(buf, data, m_size);
	}
};

enum class PlayerMoveDir : BYTE {
	LEFT = 0,
	RIGHT,
	UP,
	DOWN
};

struct PlayerInfo {
	UINT id;
	POINT pos;
};

class ClientInfo
{
public:
	ClientInfo();
	ClientInfo(SOCKET sock, WSAOVERLAPPED* over);
	~ClientInfo();
private:
	SOCKET sock;
	int s_id = 0;
	WSAOVERLAPPED* m_WSAover;
	char buf[BUFSIZE];

	WSABUF wsabuf[1];
	vector<BYTE> m_SendReserveList;

	bool m_bUpdated;
public:
	SOCKET GetSock();
	LPWSAOVERLAPPED GetWSAoverlapped();

	void Recv();
	void Send();
	void Exit();

	PlayerInfo playerinfo;
	bool IsUpdated() { return m_bUpdated; }

	void PlayerMove(PlayerMoveDir dir);
	void WriteData();
	void UpdateData(DWORD transfer_size);

};

