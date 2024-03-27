#pragma once
#include "stdafx.h"

#define serverFramework Server::GetInstance()

//constexpr char SERVER_ADDR[] = "127.0.0.1";
constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

// ���� Ŀ�ǵ� ������ 256�� �̸��ϰ���.
enum class GameCommandList : BYTE {
	NONE = 0,
	MOVE
};


enum class PlayerMoveDir : BYTE {
	LEFT = 0,
	RIGHT,
	UP,
	DOWN
};

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

	// �������� �����͸� �ְ�޴� �۾��� �ϴ� �Լ�
	void Logic();

	void Send();
	void Recv();
	CHAR* GetRecvBuffer();

	void SendReserve(void* data, size_t size);
	void ResetSendList();

	void error_display(const char* msg, int err_no);

protected:
	SOCKET m_ServerSock;
	WSABUF wsabuf[1];

	vector<BYTE> m_SendReserveList;
};

