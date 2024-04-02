#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <unordered_map>
#include <cassert>
#pragma comment (lib,"WS2_32.LIB")

#define serverFramework Server::GetInstance()

constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

using namespace std;

// ���� Ŀ�ǵ� ������ 256�� �̸��ϰ���.
enum class GameCommandList : BYTE {
	NONE = 0,
	MOVE
};

class ClientInfo;
struct PlayerInfo;

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

	void Init();
	void Accept();

	void ClientExit(LPWSAOVERLAPPED wsaover);
	static void error_display(const char* msg, int err_no);


	void WriteToBuffer(vector<BYTE>& buffer,void* data, size_t size);
protected:
	SOCKET m_ListenSock;
	unordered_map<LPWSAOVERLAPPED, ClientInfo> m_umClientInfos;
	UINT m_iClient_count = 0; //������ ������ �ƴ� �����ߴ� Ŭ���� ����

	vector<BYTE> m_CmdList;

	vector<PlayerInfo*> m_playerinfos;
public:
	unordered_map<LPWSAOVERLAPPED, ClientInfo>& GetClientInfos() { return m_umClientInfos; }

	void WriteServerBuffer(vector<BYTE>& buffer, PlayerInfo* playerinfo);
	
};
