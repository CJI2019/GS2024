#pragma once
#include "DataBase.h"
#include "ClientInfo.h"

#define serverFramework Server::GetInstance()


class Server
{
private:
	Server();
public:
	static Server& GetInstance() {
		static Server instance; // ���� ������ �ν��Ͻ��� ����, ���� �ʱ�ȭ
		return instance;
	}
	~Server();
	static void Worker_Thread();
	static int Get_new_clientId();
	static void Accept_Logic(OVER_ALLOC* o_alloc);

	void Init();
	void Accept();

	static void Disconnect(int c_id);
	static void error_display(const char* msg, int err_no);
protected:
	static SOCKET m_Server_sock;
	static SOCKET m_cSock;
	static OVER_ALLOC m_over;
	static HANDLE m_hiocp;

	static std::array<ClientInfo, MAX_USER> m_aClientInfos;

	UINT m_iClient_count = 0; //������ ������ �ƴ� �����ߴ� Ŭ���� ����

	Sector m_Sector;
public:
	std::array<ClientInfo, MAX_USER>& GetClientInfo() { return m_aClientInfos; }

	Sector& GetSector() { return m_Sector; }
};
