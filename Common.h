#pragma once
#include "../../Protocol.h"
#include <random>

constexpr int BUFSIZE = 256;

#define ROW_X 2000 // 정사각형이므로 하나만 사용해도 무방
#define COL_Y 2000
#define VIEW_RANGE 8*8

#define Rd Random_Device::GetInstance()

enum class MoveDir : BYTE {
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

struct GameInfo {
	Vector2 m_pos;
	MoveDir m_cur_direction;
	int m_visual;
};


enum class IO_TYPE : BYTE { IO_ACCEPT, IO_RECV, IO_SEND, IO_NPC_MOVE};
enum class STATE : BYTE { ST_FREE, ST_ALLOC, ST_INGAME };
enum class TIMER_EVENT_TYPE: BYTE {TE_RANDOM_MOVE};

class OVERLAPPED_EX {
public:
	WSAOVERLAPPED over;
	WSABUF m_wsabuf;
	unsigned char send_buf[BUFSIZE];
	IO_TYPE io_type;

	OVERLAPPED_EX()
	{
		m_wsabuf.len = BUFSIZE;
		m_wsabuf.buf = reinterpret_cast<CHAR*>(send_buf);
		io_type = IO_TYPE::IO_RECV;
		ZeroMemory(&over, sizeof(over));
	}
	OVERLAPPED_EX(char* packet)
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


class Random_Device {
private:
	Random_Device(){}
public:
	static Random_Device& GetInstance() {
		static Random_Device rd;
		return rd;
	}

	std::random_device rd;
	int Generate_Random_int(int num) {
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis(0, num - 1);

		return dis(gen);
	}
};