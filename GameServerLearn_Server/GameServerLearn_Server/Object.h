#pragma once
#include "CLua.h"

class Object
{
public:
	Object();
	~Object();

public:
	int				m_id;
	STATE			m_cur_state;
	std::mutex		m_mtxlock;

	//GameInfo
	std::string		m_name;
	GameInfo m_gameinfo;
	std::unordered_set<int> m_viewList;
	std::mutex m_mtxView;
	Vector2 m_sector_Pos;

	bool m_isNPC;

	//Lua
	CLua m_cLua;
public:
	virtual void Exit() {}
	virtual void Send(void* packet) {};
	virtual void Send_login() {};
	virtual void Send_move_player(void* packet) {};

	// 플레이어 이외에 객체들도 View를 넣어줘야 함.
	virtual void Send_add_object(const int& c_id, const int& visual);
	virtual void Send_remove_player(int c_id);

	//Sector
	void SectorChangeCheck(); // Sector 변경 체크
	void SectorMove(MoveDir dir);

	//Game
	bool InViewRange(int c_id);
    virtual void Move(char dir);
	virtual void Attack() = 0;
};

