#pragma once
#include "stdafx.h"

#include "Server.h"
#include "ClientInfo.h"
#include "DataBase.h"

ClientInfo::ClientInfo() : Object()
{
    m_prev_remain_byte = 0;

    m_LastMoveTime = 0;
}

ClientInfo::ClientInfo(SOCKET s, WSAOVERLAPPED* over) : ClientInfo()
{
	m_sock = s;
    //m_WSAover = over;
    m_wsabuf[0].buf = m_cbuf;
    m_wsabuf[0].len = BUFSIZE;
}

ClientInfo::~ClientInfo()
{
    closesocket(m_sock);
    //delete m_WSAover;
}

SOCKET ClientInfo::GetSock()
{
	return m_sock;
}

LPWSAOVERLAPPED ClientInfo::GetWSAoverlapped()
{
	return &m_over_alloc.over;
}

void ClientInfo::Recv()
{
    DWORD recv_flag = 0;
    ZeroMemory(&m_over_alloc.over, sizeof(m_over_alloc.over));
    m_over_alloc.m_wsabuf.len = BUFSIZE - m_prev_remain_byte;
    m_over_alloc.m_wsabuf.buf = reinterpret_cast<CHAR*>(m_over_alloc.send_buf + m_prev_remain_byte);
    int res = WSARecv(m_sock, &m_over_alloc.m_wsabuf, 1, 0, &recv_flag,
        &m_over_alloc.over, 0);
    //std::cout << "수신\n";
    //cout << "받은 데이터 => " << recv_size << endl;
    if (res != 0) {
        auto err_no = WSAGetLastError();
        if (WSA_IO_PENDING != err_no) {
            Server::error_display("WSARecv Error : ", WSAGetLastError());
            Exit();
            //assert(0);
        }
    }
}

void ClientInfo::Send(void* packet)
{
    auto b = new OVERLAPPED_EX(reinterpret_cast<char*>(packet));
    int res = WSASend(m_sock, &b->m_wsabuf, 1, nullptr, 0, &b->over, 0);

}

bool ClientInfo::ProcessPacket(char* packet)
{
    switch (packet[2])
    {
    case CS_LOGIN: {
        CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
        m_name = p->name;
        //std::cout << "[" << m_id << "] 로그인\n";
        Send_login();
        {
            std::lock_guard<std::mutex> ll(m_mtxlock);
            m_cur_state = STATE::ST_INGAME;
        }
        auto& infos = serverFramework.GetObjectInfos();
        for (auto& cl : infos) {
            {
                std::lock_guard<std::mutex> ll(cl->m_mtxlock);
                if (cl->m_cur_state != STATE::ST_INGAME) continue;
            }
            if (cl->m_id == m_id) continue;
            if (!InViewRange(cl->m_id)) continue; // 시야에 있지 않으면 추가하지 않음.
            if (cl->m_isNPC) {
                Send_add_object(cl->m_id,cl->m_gameinfo.m_visual);
                cl->Send_add_object(m_id, m_gameinfo.m_visual);
                continue;
            }
            cl->Send_add_object(m_id, m_gameinfo.m_visual);

            Send_add_object(cl->m_id, cl->m_gameinfo.m_visual);
		}
        break;
    }
    case CS_MOVE: {
        move_count++;
        CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

        //m_mtxlock.lock();
        m_LastMoveTime = p->move_time;
        m_gameinfo.m_cur_direction = static_cast<MoveDir>(p->direction);
        Move(p->direction);

        Send_move_player(p);
        //m_mtxlock.unlock();

        //std::cout << "[" << m_id << "] MOVE\n";
        //std::cout << move_count << "번 움직임\n";
        //std::cout << m_gameinfo.m_pos.x << "," << m_gameinfo.m_pos.y << std::endl;
        break;
    }
    case CS_TELEPORT: {
        CS_TELEPORT_PACKET* p = reinterpret_cast<CS_TELEPORT_PACKET*>(packet);
        m_gameinfo.m_pos.x = Rd.Generate_Random_int(ROW_X);
        m_gameinfo.m_pos.y = Rd.Generate_Random_int(COL_Y);

        Sector& sectors = serverFramework.GetSector();
        if (!sectors.InCurrentSector(m_gameinfo.m_pos, m_sector_Pos)) {
            sectors.PopSectorId(m_sector_Pos, m_id);
            m_sector_Pos = sectors.AllocSectorId(m_gameinfo.m_pos, m_id);
        }
        Send_move_player(nullptr);
        break;
    }
    case CS_LOGOUT: {
        Exit();
        return false;
    }
    default:
        break;
    }
    return true;
}

void ClientInfo::Exit()
{
    DB_ID_INFO d_data;
    d_data.m_strUserid = m_name;
    d_data.m_xPos = m_gameinfo.m_pos.x;
    d_data.m_yPos = m_gameinfo.m_pos.y;
    d_data.m_hp = m_gameinfo.m_hp;
    d_data.m_maxhp = m_gameinfo.m_maxhp;
    d_data.m_level = m_gameinfo.m_level;
    d_data.m_exp = m_gameinfo.m_exp;
    DB.DB_Process(&d_data, EXIT);

    serverFramework.Disconnect(m_id);
}

void ClientInfo::Send_login()
{
    SC_LOGIN_INFO_PACKET sc_p;
    
    sc_p.type = SC_LOGIN_INFO;
    sc_p.id = m_id;

    // 1. DB에서 ID 정보가 있는지 확인한다.
    DB_ID_INFO d_data;
    d_data.m_strUserid = m_name;
    DB.DB_Process(&d_data, LOGIN);
    if (d_data.m_xPos == -1) { // 2. 정보가 없다면 DB에 ID 등록.
        d_data.m_xPos = Rd.Generate_Random_int(ROW_X);
        d_data.m_yPos = Rd.Generate_Random_int(COL_Y);
        d_data.m_hp = 100;
        d_data.m_maxhp = 100;
        d_data.m_level = 1;
        d_data.m_exp = 0;
        DB.DB_Process(&d_data, REGISTER);
    }

    m_gameinfo.m_pos.x = static_cast<short>(d_data.m_xPos);
    m_gameinfo.m_pos.y = static_cast<short>(d_data.m_yPos);
    m_gameinfo.m_exp = d_data.m_exp;
    m_gameinfo.m_hp = d_data.m_hp;
    m_gameinfo.m_maxhp = d_data.m_maxhp;
    m_gameinfo.m_level = d_data.m_level;

    for (;;) {
        if (m_gameinfo.m_exp >= m_gameinfo.m_maxexp) {
            m_gameinfo.m_maxexp *= 2;
            continue;
        }
        break;
    }

    if (m_gameinfo.m_pos.x < 0 || m_gameinfo.m_pos.y < 0) {
        serverFramework.Disconnect(m_id);
        return;
        //assert(0);
    }
    //m_gameinfo.m_pos.x = 0;
    //m_gameinfo.m_pos.y = 0;
    m_sector_Pos = serverFramework.GetSector().AllocSectorId(m_gameinfo.m_pos, m_id);

    sc_p.exp = m_gameinfo.m_exp;
    sc_p.hp = m_gameinfo.m_hp;
    sc_p.max_hp = m_gameinfo.m_maxhp;
    sc_p.level = m_gameinfo.m_level;
    sc_p.x = m_gameinfo.m_pos.x;
    sc_p.y = m_gameinfo.m_pos.y;
    sc_p.size = sizeof(SC_LOGIN_INFO_PACKET);

    Send(&sc_p);
    //std::cout << "Send 로그인\n";
    auto& sf = serverFramework;
    sf.PushTimer(TIMER_EVENT_TYPE::TE_ATTACK, m_id,1s);
}

void ClientInfo::Send_move_player(void* packet) // 굳이 매개변수를 받을 필요는 현재까지는 없음.
{
    SC_MOVE_OBJECT_PACKET sc_p;

    sc_p.type = SC_MOVE_OBJECT;
    sc_p.id = m_id;
    sc_p.x = m_gameinfo.m_pos.x;
    sc_p.y = m_gameinfo.m_pos.y;
    sc_p.size = sizeof(SC_MOVE_OBJECT_PACKET);
    sc_p.move_time = m_LastMoveTime;

    Send(&sc_p);

    auto& infos = serverFramework.GetObjectInfos();

    m_mtxView.lock();
    std::unordered_set<int> old_View(m_viewList);
    m_mtxView.unlock();
    std::unordered_set<int> new_View;

    SectorChangeCheck();
    auto& sectors = serverFramework.GetSector();

    // Sector의 경계선에서 view에 담길 수 있는 다른 Sector의 객체도 보이도록한다.
    std::unordered_set<int> cur_sector = sectors.GetCurrentSector(m_sector_Pos);
    //std::cout << "cur_sector에 존재하는 객체의 개수 => " << cur_sector.size() << std::endl;
    // 같은 섹터 내에서만 보이는 객체를 선별하도록 한다.
    for (auto& c_id : cur_sector) { // 보이는 객체 선별
        if (infos[c_id]->m_cur_state != STATE::ST_INGAME) continue;
        if (infos[c_id]->m_id == m_id) continue;
        if (InViewRange(infos[c_id]->m_id)) {
            new_View.insert(infos[c_id]->m_id);
        }
    }

    for (auto& c_id : new_View) { // Player 추가 작업
        if (infos[c_id]->m_cur_state != STATE::ST_INGAME) continue;
        if (infos[c_id]->m_id == m_id) continue;

        infos[c_id]->m_mtxView.lock();
        if (infos[c_id]->m_viewList.count(m_id)) { // 상대방의 View에 존재할때
            infos[c_id]->m_mtxView.unlock();
            infos[c_id]->Send(&sc_p);
        }
        else {
            infos[c_id]->m_mtxView.unlock();
            infos[c_id]->Send_add_object(m_id, m_gameinfo.m_visual);
        }
        
        Send_add_object(c_id, infos[c_id]->m_gameinfo.m_visual);
    }

    for (auto& c_id : old_View) {
        if (infos[c_id]->m_cur_state != STATE::ST_INGAME) {
            // old_View 에 존재 했으나 좀비 상태가 되어 있을 수 있음.
            // 상대방은 종료를 했는데 view에 남아 있게 되면 안됨.
            Send_remove_player(c_id);
            continue;
        }
        if (infos[c_id]->m_id == m_id) continue;

        if (new_View.count(c_id) == 0) { // 최신의 new_View에 old_View의 요소가 없을경우 
            Send_remove_player(c_id);
            infos[c_id]->Send_remove_player(m_id);
        }
    }
}

void ClientInfo::Send_add_object(const int& c_id, const int& visual)
{
    m_mtxView.lock();
    if (m_viewList.count(c_id) == 0) {
        m_viewList.insert(c_id); // unordered_set을 이용하므로 원소는 유일함.
    }
    else {
        m_mtxView.unlock();
        return;
    }
    m_mtxView.unlock();

    SC_ADD_OBJECT_PACKET sc_p;

    sc_p.type = SC_ADD_OBJECT;
    sc_p.id = c_id;
    sc_p.size = sizeof(SC_ADD_OBJECT_PACKET);
    sc_p.visual = visual;
    auto& cl_infos = serverFramework.GetObjectInfos();

    strncpy(sc_p.name, cl_infos[c_id]->m_name.c_str(), cl_infos[c_id]->m_name.size() + 1);
    sc_p.x = cl_infos[c_id]->m_gameinfo.m_pos.x;
    sc_p.y = cl_infos[c_id]->m_gameinfo.m_pos.y;

    Send(&sc_p);
    //std::cout << "[" << m_id << "] 에게 Send Add [" << c_id << "] Pos(" << sc_p.x << "," << sc_p.y << ")" << std::endl;
}

void ClientInfo::Send_remove_player(int c_id)
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
    SC_REMOVE_OBJECT_PACKET sc_p;

    sc_p.type = SC_REMOVE_OBJECT;
    sc_p.id = c_id;
    sc_p.size = sizeof(SC_REMOVE_OBJECT_PACKET);

    Send(&sc_p);
}

void ClientInfo::Send_stat_change()
{
    SC_STAT_CHANGE_PACKET sc_p;
    sc_p.exp = m_gameinfo.m_exp;
    sc_p.hp = m_gameinfo.m_hp;
    sc_p.max_hp = m_gameinfo.m_maxhp;
    sc_p.level = m_gameinfo.m_level;
    sc_p.size = sizeof(SC_STAT_CHANGE_PACKET);
    sc_p.type = SC_STAT_CHANGE;

    Send(&sc_p);
}

void ClientInfo::Move(char dir)
{
    Object::Move(dir);
}

void ClientInfo::Attack()
{
    auto& obj_infos = serverFramework.GetObjectInfos();

    std::unordered_set<int> cur_view;
    {
        lock_guard<mutex> vl(m_mtxView);
        cur_view = m_viewList;
    }


    int nearest_id = -1;
    int near_range = 99999999;
    for (auto& obj_id : cur_view) {
        if (obj_id < MAX_USER) continue; // NPC 만 찾도록한다.
        if (obj_infos[obj_id]->m_cur_state != STATE::ST_INGAME) continue;
        if (obj_id == m_id) continue;
        auto other_pos = obj_infos[obj_id]->m_gameinfo.m_pos;
        auto my_pos = m_gameinfo.m_pos;
        Vector2 AtoB;
        AtoB.x = other_pos.x - my_pos.x;
        AtoB.y = other_pos.y - my_pos.y;
        int range = (AtoB.x * AtoB.x) + (AtoB.y * AtoB.y);
        if (near_range > range) {
            near_range = range;
            nearest_id = obj_id;
        }
    }

    if (nearest_id != -1) {
        SC_CHAT_PACKET sc_p;
        sc_p.id = m_id;
        sc_p.type = SC_CHAT;

        int damege = m_gameinfo.m_level * 10;

        {
            lock_guard<mutex> ll(obj_infos[nearest_id]->m_gameinfo.m_mtxhp);
            if (obj_infos[nearest_id]->m_gameinfo.m_hp - damege > 0) {
                obj_infos[nearest_id]->m_gameinfo.m_hp -= damege;
            }
            else {
                //damege = obj_infos[nearest_id]->m_gameinfo.m_hp;
                obj_infos[nearest_id]->m_gameinfo.m_hp = 0;
            }
        }

        sprintf_s(sc_p.mess, CHAT_SIZE, "%s 가 Lv.%d - %s 를 %d의 데미지로 공격하였습니다.\n%d의 hp가 남았습니다."
            , m_name.c_str(), obj_infos[nearest_id]->m_gameinfo.m_level, obj_infos[nearest_id]->m_name.c_str(), damege, obj_infos[nearest_id]->m_gameinfo.m_hp);
        size_t str_size = strlen(sc_p.mess);
        sc_p.mess[str_size] = '\0';
        sc_p.size = sizeof(SC_CHAT_PACKET) - (CHAT_SIZE - static_cast<short>(str_size + 1));
        Send(&sc_p);

        obj_infos[nearest_id]->m_gameinfo.m_mtxhp.lock();
        if (obj_infos[nearest_id]->m_gameinfo.m_hp < 1) {
            obj_infos[nearest_id]->m_gameinfo.m_hp = obj_infos[nearest_id]->m_gameinfo.m_maxhp;
            obj_infos[nearest_id]->m_gameinfo.m_mtxhp.unlock();
            int exp = obj_infos[nearest_id]->m_gameinfo.m_level * obj_infos[nearest_id]->m_gameinfo.m_level * 2;
            m_gameinfo.m_exp += exp;
            CalculateMaxExp();
           
            sprintf_s(sc_p.mess, CHAT_SIZE, "%s 를 처치하여 경험치 %d 를 얻었습니다."
                , obj_infos[nearest_id]->m_name.c_str(), exp);

            size_t str_size = strlen(sc_p.mess);
            sc_p.mess[str_size] = '\0';
            sc_p.size = sizeof(SC_CHAT_PACKET) - (CHAT_SIZE - static_cast<short>(str_size + 1));
            Send(&sc_p);
            Send_stat_change();
        }
        else {
            obj_infos[nearest_id]->m_gameinfo.m_mtxhp.unlock();
        }
    }

    
    {
        m_mtxlock.lock();
        if (m_cur_state == STATE::ST_INGAME) {
            m_mtxlock.unlock();
            serverFramework.PushTimer(TIMER_EVENT_TYPE::TE_ATTACK, m_id, 1s);
            return;
        }
        m_mtxlock.unlock();
    }
}

void ClientInfo::CalculateMaxExp()
{
    for (;;) {
        if (m_gameinfo.m_exp >= m_gameinfo.m_maxexp) {
            m_gameinfo.m_level += 1;
            m_gameinfo.m_maxexp *= 2;
            m_gameinfo.m_maxhp += 100;
            m_gameinfo.m_hp = m_gameinfo.m_maxhp;
            continue;
        }
        break;
    }
}