#pragma once
#include "stdafx.h"

#define serverFramework Server::GetInstance()

//constexpr char SERVER_ADDR[] = "127.0.0.1";
constexpr short SERVER_PORT = 4000;
constexpr int BUFSIZE = 256;

// 게임 커맨드 개수는 256개 미만일것임.
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
		static Server instance; // 정적 변수로 인스턴스를 유지, 지연 초기화
		return instance;
	}
	~Server();

	// 서버에서 데이터를 주고받는 작업을 하는 함수
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

