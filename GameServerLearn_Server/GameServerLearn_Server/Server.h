#pragma once
#include "../../Protocol.h"

#define serverFramework Server::GetInstance()


class ClientInfo;

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
	void ClientExit(LPWSAOVERLAPPED wsaover);
	static void error_display(const char* msg, int err_no);


	void WriteToBuffer(std::vector<BYTE>& buffer,void* data, size_t size);
protected:
	static SOCKET m_Server_sock;
	static SOCKET m_cSock;
	static OVER_ALLOC m_over;
	static HANDLE m_hiocp;

	static std::array<ClientInfo, MAX_USER> m_aClientInfos;


	std::unordered_map<LPWSAOVERLAPPED, ClientInfo> m_umClientInfos;
	UINT m_iClient_count = 0; //������ ������ �ƴ� �����ߴ� Ŭ���� ����

	std::vector<BYTE> m_CmdList;

	std::vector<PlayerInfo*> m_playerinfos;
public:
	std::array<ClientInfo, MAX_USER>& GetClientInfo() { return m_aClientInfos; }

	void WriteServerBuffer(std::vector<BYTE>& buffer, PlayerInfo* playerinfo);
	
};
