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
		static Server instance; // 정적 변수로 인스턴스를 유지, 지연 초기화
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
	UINT m_iClient_count = 0; //접속한 개수가 아닌 접속했던 클라의 개수

	std::vector<BYTE> m_CmdList;

	std::vector<PlayerInfo*> m_playerinfos;
public:
	std::array<ClientInfo, MAX_USER>& GetClientInfo() { return m_aClientInfos; }

	void WriteServerBuffer(std::vector<BYTE>& buffer, PlayerInfo* playerinfo);
	
};
