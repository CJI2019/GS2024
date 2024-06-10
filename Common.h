#pragma once
#include "stdafx.h"

constexpr int BUFSIZE = 256;

#define ROW_X 2000 // 정사각형이므로 하나만 사용해도 무방
#define COL_Y 2000

enum class PlayerMoveDir : BYTE {
	UP = 0,
	DOWN,
	LEFT,
	RIGHT,
	NONE,
};

struct Vector2 {
	short x;
	short y;

	Vector2() {
		x = 0;
		y = 0;
	}
	Vector2(short xpos, short ypos) {
		x = xpos;
		y = ypos;
	}
};

struct PlayerInfo {
	Vector2 pos;
};


enum class IO_TYPE : BYTE { IO_ACCEPT, IO_RECV, IO_SEND };
enum class STATE : BYTE { ST_FREE, ST_ALLOC, ST_INGAME };

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
