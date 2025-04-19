#pragma once
#include "stdafx.h"
#include "NPC.h"
#include "Server.h"

NPC::NPC() : Object()
{
	m_gameinfo.m_pos.x = Rd.Generate_Random_int(ROW_X);
	m_gameinfo.m_pos.y = Rd.Generate_Random_int(COL_Y);

	m_isNPC = true;
	m_isActive = false;

	m_gameinfo.m_level = Rd.Generate_Random_int(100);
}

NPC::~NPC()
{
}

void NPC::Send_add_object(const int& c_id, const int& visual)
{
	Object::Send_add_object(c_id, visual);

	bool comp = false;
	if (std::atomic_compare_exchange_strong(&m_isActive, &comp, true) == false) {
		return; // true 만드는데 실패 했다면.? (이미 다른 쓰레드가 true 로 만든것임)
	}

	auto& sf = serverFramework;
	sf.PushTimer(TIMER_EVENT_TYPE::TE_NPC_RANDOM_MOVE, m_id,1s);
}

void NPC::Move(char dir)
{
	m_gameinfo.m_cur_direction = static_cast<MoveDir>(rand() % 4);
	Object::Move(static_cast<char>(m_gameinfo.m_cur_direction));
	
	SectorChangeCheck();

	SC_MOVE_OBJECT_PACKET sc_p;

	sc_p.type = SC_MOVE_OBJECT;
	sc_p.id = m_id;
	sc_p.x = m_gameinfo.m_pos.x;
	sc_p.y = m_gameinfo.m_pos.y;
	sc_p.size = sizeof(SC_MOVE_OBJECT_PACKET);

	auto& infos = serverFramework.GetObjectInfos();
	m_mtxView.lock();
	auto old_View = m_viewList;
	m_mtxView.unlock();
	std::unordered_set<int> new_View;

	// 보이는 객체 선별
	// NPC는 플레이어의 시야에 들어오면 동작을 하기 때문에
	// 섹터에서 검색을 할 필요는 없다.
	// 자신을 시야에 둔 플레이어에게만 신경 쓰면됨.
	// 단, 이동시에 섹터 이동 또한 필요하다.
	// 플레이어는 섹터를 기준으로 View를 만든다.
	auto& sectors = serverFramework.GetSector();
	std::unordered_set<int> cur_sector = sectors.GetRangeSector(m_sector_Pos);
	for (auto& c_id : cur_sector) { // 보이는 객체 선별
		if (infos[c_id]->m_cur_state != STATE::ST_INGAME) continue;
		if (c_id == m_id) continue;
		if (c_id >= MAX_USER) { continue; }
		if (InViewRange(c_id) && old_View.count(c_id) == 0) {
			infos[c_id]->Send_add_object(m_id, m_gameinfo.m_visual);
			Send_add_object(c_id, infos[c_id]->m_gameinfo.m_visual);
			new_View.insert(c_id);
		}
	}

	for (auto& cl_id : old_View) {
		if (m_cur_state != STATE::ST_INGAME) continue;
		// NPC 객체와 id가 겹칠일은 없지만 
		// 나중에 충돌처리 등 다른 npc의 정보가 필요할때 바뀔 것을 대비함.
		if (cl_id == m_id) continue; 
		if (!InViewRange(cl_id)) continue;

		new_View.insert(cl_id);
	}

	// 보이는 객체에게 Move 정보 Send
	for (auto& cl_id : new_View) {
		if (m_cur_state != STATE::ST_INGAME) continue;
		if (cl_id == m_id) continue;
		infos[cl_id]->Send(&sc_p);
	}

	// 보이지 않는 객체에게서 this 객체 지우기
	for (auto& cl_id : old_View) {
		if (m_cur_state != STATE::ST_INGAME) {
			Send_remove_player(cl_id);
			continue;
		}
		if (cl_id == m_id) continue;
		if (new_View.count(cl_id) == 0) {
			Send_remove_player(cl_id);
			infos[cl_id]->Send_remove_player(m_id);
		}
	}

	if (new_View.size() != 0) {
		serverFramework.PushTimer(TIMER_EVENT_TYPE::TE_NPC_RANDOM_MOVE, m_id,1s);
	}
	else {
		bool comp = true;
		if (std::atomic_compare_exchange_strong(&m_isActive, &comp, false) == false) {
			return;
		}
	}
}
