#pragma once
#include "stdafx.h"
#include "Server.h"
#include "Object.h"

Object::Object()
{
    m_id = -1;

    m_gameinfo.m_pos = { 0,0 };
    m_gameinfo.m_exp = 0;
    m_gameinfo.m_maxexp = 100;
    m_gameinfo.m_level = 1;
    m_gameinfo.m_hp = 100;
    m_gameinfo.m_maxhp = 100;
    
    m_cur_state = STATE::ST_FREE;

    m_isNPC = false;

}

Object::~Object()
{
}

void Object::Send_add_object(const int& c_id, const int& visual)
{
    m_mtxView.lock();
    m_viewList.insert(c_id); // unordered_set을 이용하므로 원소는 유일함.
    m_mtxView.unlock();
}

void Object::Send_remove_player(int c_id)
{
    m_mtxView.lock();
    if (m_viewList.count(c_id) != 0) {
        m_viewList.erase(c_id);
    }
    else {
        m_mtxView.unlock();
        return;
    }
    m_mtxView.unlock();
}

bool Object::InViewRange(int c_id)
{
    auto& infos = serverFramework.GetObjectInfos();

    int Cx = infos[c_id]->m_gameinfo.m_pos.x - m_gameinfo.m_pos.x;
    int Cy = infos[c_id]->m_gameinfo.m_pos.y - m_gameinfo.m_pos.y;

    return Cx * Cx + Cy * Cy < VIEW_RANGE;
}
void Object::SectorChangeCheck()
{
    Sector& sectors = serverFramework.GetSector();
    if (!sectors.InCurrentSector(m_gameinfo.m_pos, m_sector_Pos)) {
        // 현재 섹터에 없다면 섹터를 갱신해줘야함.
        sectors.PopSectorId(m_sector_Pos, m_id);
        //Dir로 움직였기 떄문에 섹터가 변한 것이므로 섹터를 한칸 이동한다.
        SectorMove(m_gameinfo.m_cur_direction);
        sectors.PushSectorId(m_sector_Pos, m_id);
    }
}

void Object::SectorMove(MoveDir dir)
{
    switch (dir)
    {
    case MoveDir::UP:
        m_sector_Pos.y -= 1;
        break;
    case MoveDir::DOWN:
        m_sector_Pos.y += 1;
        break;
    case MoveDir::LEFT:
        m_sector_Pos.x -= 1;
        break;
    case MoveDir::RIGHT:
        m_sector_Pos.x += 1;
        break;
    case MoveDir::NONE:
        break;
    default:
        assert(0); // 잘못된 Dir
        break;
    }
}

void Object::Move(char dir)
{
    switch (static_cast<MoveDir>(dir))
    {
    case MoveDir::LEFT:
        if (0 < m_gameinfo.m_pos.x) {
            m_gameinfo.m_pos.x -= 1;
        }
        //cout << "왼쪽" << endl;
        break;
    case MoveDir::RIGHT:
        if (m_gameinfo.m_pos.x < ROW_X - 1) {
            m_gameinfo.m_pos.x += 1;
        }
        //cout << "오른쪽" << endl;
        break;
    case MoveDir::UP:
        if (0 < m_gameinfo.m_pos.y) {
            m_gameinfo.m_pos.y -= 1;
        }
        //cout << "위" << endl;
        break;
    case MoveDir::DOWN:
        if (m_gameinfo.m_pos.y < COL_Y - 1) {
            m_gameinfo.m_pos.y += 1;
        }
        //cout << "아래" << endl;
        break;
    default:
        assert(0);
    }
}