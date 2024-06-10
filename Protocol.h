#pragma once

constexpr int PORT_NUM = 4000;
constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 300;

constexpr int MAX_USER = 10000;
constexpr int MAX_NPC = 200000;


// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_CHAT = 2;
constexpr char CS_ATTACK = 3;			// 4 ���� ����
constexpr char CS_TELEPORT = 4;			// RANDOM�� ��ġ�� Teleport, Stress Test�� �� Hot Spot������ ���ϱ� ���� ����
constexpr char CS_LOGOUT = 5;			// Ŭ���̾�Ʈ���� ���������� ������ �����ϴ� ��Ŷ

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_LOGIN_FAIL = 3;
constexpr char SC_ADD_OBJECT = 4;
constexpr char SC_REMOVE_OBJECT = 5;
constexpr char SC_MOVE_OBJECT = 6;
constexpr char SC_CHAT = 7;
constexpr char SC_STAT_CHANGE = 8;


#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned short size;
	char	type;
	char	name[NAME_SIZE];
};

struct CS_MOVE_PACKET {
	unsigned short size;
	char	type;
	char	direction;  // 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
	unsigned	move_time;
};

struct CS_CHAT_PACKET {
	unsigned short size;			// ũ�Ⱑ �����̴�, mess�� ������ size�� ������.
	char	type;
	char	mess[CHAT_SIZE];
};

struct CS_TELEPORT_PACKET {			// �������� �ڷ���Ʈ �ϴ� ��Ŷ, ���� �׽�Ʈ�� �ʿ�
	unsigned short size;
	char	type;
};

struct CS_LOGOUT_PACKET {
	unsigned short size;
	char	type;
};

struct SC_LOGIN_INFO_PACKET {
	unsigned short size;
	char	type;
	int		visual;				// ����, �������� ������ �� ���
	int		id;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;
	short	x, y;
};

struct SC_ADD_OBJECT_PACKET {
	unsigned short size;
	char	type;
	int		id;
	int		visual;				// ��� ���� OBJECT�ΰ��� ����
	short	x, y;
	char	name[NAME_SIZE];
};

struct SC_REMOVE_OBJECT_PACKET {
	unsigned short size;
	char	type;
	int		id;
};

struct SC_MOVE_OBJECT_PACKET {
	unsigned short size;
	char	type;
	int		id;
	short	x, y;
	unsigned int move_time;
};

struct SC_CHAT_PACKET {
	unsigned short size;
	char	type;
	int		id;
	char	mess[CHAT_SIZE];
};

struct SC_LOGIN_FAIL_PACKET {
	unsigned short size;
	char	type;
};

struct SC_STAT_CHANGE_PACKET {
	unsigned short size;
	char	type;
	int		hp;
	int		max_hp;
	int		exp;
	int		level;

};

#pragma pack (pop)
//#pragma pack (push, 1)
//// ���� Ŀ�ǵ� ������ 256�� �̸��ϰ���.
//enum class Packet_Id : BYTE {
//	NONE = 0,
//	LOGIN,
//	MOVE,
//	ADD_PLAYER
//};
//
//
//enum class PlayerMoveDir : BYTE {
//	NONE = 0,
//	LEFT,
//	RIGHT,
//	UP,
//	DOWN
//};
//
//
//struct CS_NONE_PACKET {
//	unsigned char size;
//	Packet_Id type = Packet_Id::NONE;
//};
//
//struct CS_LOGIN_PACKET {
//	unsigned char size;
//	Packet_Id type = Packet_Id::LOGIN;
//};
//
//struct CS_MOVE_PACKET {
//	unsigned char size;
//	Packet_Id type = Packet_Id::MOVE;
//
//	PlayerMoveDir Dir = PlayerMoveDir::NONE;
//};
//
//struct SC_NONE_TYPE_PACKET {
//	unsigned char size;
//	Packet_Id type = Packet_Id::NONE;
//};
//
//struct SC_LOGIN_PACKET {
//	unsigned char size;
//	Packet_Id type = Packet_Id::LOGIN;
//	UINT id;
//	POINT pos;
//};
//
//struct SC_MOVE_PLAYER_PACKET {
//	unsigned char size;
//	Packet_Id type = Packet_Id::MOVE;
//	UINT id;
//	POINT pos;
//};
//
//struct SC_ADD_PLAYER_PACKET {
//	unsigned char size;
//	Packet_Id type = Packet_Id::ADD_PLAYER;
//	UINT id;
//	POINT pos;
//};
//
//struct PlayerInfo {
//	POINT pos;
//};
//
//#pragma pack (pop)

