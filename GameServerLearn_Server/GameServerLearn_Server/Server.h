#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <unordered_map>
#include <cassert>
#pragma comment (lib,"WS2_32.LIB")

constexpr char SERVER_ADDR[] = "127.0.0.1";
constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

using namespace std;

// ���� Ŀ�ǵ� ������ 256�� �̸��ϰ���.
enum class GameCommandList : BYTE {
	NONE = 0,
	MOVE
};


class ClientInfo;

class Server
{
public:
	Server();
	~Server();

	// �������� �����͸� �ְ�޴� �۾��� �ϴ� �Լ�
	void Logic();
	void ClientExit(int id);

	int Send(ClientInfo& info);
	int Recv(ClientInfo& info);
	void error_display(const char* msg, int err_no);

	void ResetSendList();

	void AssembleDataFromBytes(ClientInfo& info);

	void PackDataToBytes(ClientInfo& info);

protected:
	SOCKET m_ListenSock;
	unordered_map<int, ClientInfo> m_client_Infos;
	UINT m_iClient_count = 0; //������ ������ �ƴ� �����ߴ� Ŭ���� ����

	WSABUF wsabuf[1];
	vector<BYTE> m_SendReserveList;
	vector<BYTE> m_CmdList;
};
