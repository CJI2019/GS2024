#pragma once
#include "Object.h"
#include "NPC.h"

class ClientInfo : public Object
{
public:
	// Server Send & Recv 
	SOCKET			m_sock;
	OVERLAPPED_EX		m_over_alloc;
	char			m_cbuf[BUFSIZE];
	WSABUF			m_wsabuf[1];
	std::vector<BYTE> m_SendReserveList;
	int				m_prev_remain_byte;

	// STRESS TEST
	int m_LastMoveTime; 
	
	//GameInfo
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
	void Send_add_object(const int& c_id, const int& visual);
	void Send_remove_player(int c_id);

	void Move(char dir);
};

