#pragma once

constexpr short SERVER_PORT = 9000;
constexpr int BUFSIZE = 256;

constexpr int MAX_USER = 10000;

enum class IO_TYPE : BYTE { IO_ACCEPT, IO_RECV, IO_SEND };
enum class STATE : BYTE { ST_FREE, ST_ALLOC, ST_INGAME };

#pragma pack (push, 1)
// 게임 커맨드 개수는 256개 미만일것임.
enum class GameCommand_Type : BYTE {
	NONE = 0,
	LOGIN,
	MOVE,
	ADD_PLAYER
};


enum class PlayerMoveDir : BYTE {
	NONE = 0,
	LEFT,
	RIGHT,
	UP,
	DOWN
};


struct CS_NONE_PACKET {
	unsigned char size;
	GameCommand_Type type = GameCommand_Type::NONE;
};

struct CS_LOGIN_PACKET {
	unsigned char size;
	GameCommand_Type type = GameCommand_Type::LOGIN;
};

struct CS_MOVE_PACKET {
	unsigned char size;
	GameCommand_Type type = GameCommand_Type::MOVE;

	PlayerMoveDir Dir = PlayerMoveDir::NONE;
};

struct SC_NONE_TYPE_PACKET {
	unsigned char size;
	GameCommand_Type type = GameCommand_Type::NONE;
};

struct SC_LOGIN_PACKET {
	unsigned char size;
	GameCommand_Type type = GameCommand_Type::LOGIN;
	UINT id;
	POINT pos;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	GameCommand_Type type = GameCommand_Type::MOVE;
	UINT id;
	POINT pos;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	GameCommand_Type type = GameCommand_Type::ADD_PLAYER;
	UINT id;
	POINT pos;
};

struct PlayerInfo {
	POINT pos;
};

#pragma pack (pop)


class OVER_ALLOC {
public:
	WSAOVERLAPPED over;
	WSABUF m_wsabuf;
	unsigned char send_buf[BUFSIZE];
	IO_TYPE io_type;

	OVER_ALLOC()
	{
		m_wsabuf.len = BUFSIZE;
		m_wsabuf.buf = reinterpret_cast<CHAR*>(send_buf);
		io_type = IO_TYPE::IO_RECV;
		ZeroMemory(&over, sizeof(over));
	}
	OVER_ALLOC(char* packet)
	{
		m_wsabuf.len = packet[0];
		m_wsabuf.buf = reinterpret_cast<CHAR*>(send_buf);
		io_type = IO_TYPE::IO_SEND;
		ZeroMemory(&over, sizeof(over));
		memcpy(send_buf, packet, packet[0]);
	}

	void SetSendPacket(char* packet)
	{
		m_wsabuf.len = packet[0];
		m_wsabuf.buf = reinterpret_cast<CHAR*>(send_buf);
		io_type = IO_TYPE::IO_SEND;
		ZeroMemory(&over, sizeof(over));
		memcpy(send_buf, packet, packet[0]);
	}

	void RecvPrepare(void* buffer)
	{
		m_wsabuf.len = BUFSIZE;
		m_wsabuf.buf = reinterpret_cast<CHAR*>(buffer);
		io_type = IO_TYPE::IO_RECV;
		ZeroMemory(&over, sizeof(over));
	}
};
