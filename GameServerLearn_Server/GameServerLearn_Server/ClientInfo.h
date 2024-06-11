#pragma once

class ClientInfo
{
public:
	// Server Send & Recv 
	SOCKET			m_sock;
	int				m_id = 0;
	OVER_ALLOC		m_over_alloc;
	char			m_cbuf[BUFSIZE];
	WSABUF			m_wsabuf[1];
	std::vector<BYTE> m_SendReserveList;
	int				m_prev_remain_byte;
	bool			m_bUpdated;
	STATE			m_cur_state;
	std::mutex		m_mtxlock;

	// STRESS TEST
	int m_LastMoveTime; 
	
	//GameInfo
	std::string		m_name;
	PlayerInfo playerinfo;
	int move_count{};
	std::unordered_set<int> m_viewList;
	std::mutex m_mtxView;
	
	Vector2 m_sector_Pos;
	
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
	void Send_remove_player(int c_id);

	bool IsUpdated() { return m_bUpdated; }

	void PlayerMove(char dir);
	void WriteData();

	bool InViewRange(int c_id);
};

