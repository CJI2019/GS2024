#pragma once

class ClientInfo
{
public:
	SOCKET			m_sock;
	int				m_id = 0;
	OVER_ALLOC		m_over_alloc;
	char			m_cbuf[BUFSIZE];

	WSABUF			m_wsabuf[1];
	std::vector<BYTE> m_SendReserveList;

	bool			m_bUpdated;
	STATE			m_cur_state;
	std::mutex		m_mtxlock;

	std::string		m_name;
	int				m_prev_remain_byte;

	PlayerInfo playerinfo;
	int move_count{};
public:
	ClientInfo();
	ClientInfo(SOCKET m_sock, WSAOVERLAPPED* over);
	~ClientInfo();
	SOCKET GetSock();
	LPWSAOVERLAPPED GetWSAoverlapped();

	void Recv();
	void Send(void* packet);
	void ProcessPacket(char* packet);
	void Exit();

	void Send_login();
	void Send_move_player(void* packet);
	void Send_add_player(int c_id);

	bool IsUpdated() { return m_bUpdated; }

	void PlayerMove(PlayerMoveDir dir);
	void WriteData();
	void UpdateData(DWORD transfer_size);

};

