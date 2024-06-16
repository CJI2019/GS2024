#pragma once
#include "stdafx.h"

#include "Server.h"
#include "ClientInfo.h"
#include "DataBase.h"

ClientInfo::ClientInfo()
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
    //std::cout << "����\n";
    //cout << "���� ������ => " << recv_size << endl;
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

void ClientInfo::ProcessPacket(char* packet)
{
    switch (packet[2])
    {
    case CS_LOGIN: {
        //std::cout << "[" << m_id << "] �α���\n";
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
            if (!InViewRange(cl->m_id)) continue; // �þ߿� ���� ������ �߰����� ����.
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
        //std::cout << move_count << "�� ������\n";
        //std::cout << m_gameinfo.m_pos.x << "," << m_gameinfo.m_pos.y << std::endl;
        break;
    }
    default:
        break;
    }
}

void ClientInfo::Exit()
{
    serverFramework.Disconnect(m_id);
}

void ClientInfo::Send_login()
{
    SC_LOGIN_INFO_PACKET sc_p;
    
    sc_p.type = SC_LOGIN_INFO;
    sc_p.id = m_id;

    m_gameinfo.m_pos.x = Rd.Generate_Random_int(ROW_X);
    m_gameinfo.m_pos.y = Rd.Generate_Random_int(COL_Y);
    //m_gameinfo.m_pos.x = 0;
    //m_gameinfo.m_pos.y = 0;
    m_sector_Pos = serverFramework.GetSector().AllocSectorId(m_gameinfo.m_pos, m_id);

    sc_p.x = m_gameinfo.m_pos.x;
    sc_p.y = m_gameinfo.m_pos.y;
    sc_p.size = sizeof(SC_LOGIN_INFO_PACKET);

    Send(&sc_p);
    //std::cout << "Send �α���\n";
}

void ClientInfo::Send_move_player(void* packet) // ���� �Ű������� ���� �ʿ�� ��������� ����.
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

    Sector& sectors = serverFramework.GetSector();
    if (!sectors.InCurrentSector(m_gameinfo.m_pos, m_sector_Pos)){
        // ���� ���Ϳ� ���ٸ� ���͸� �����������.
        sectors.PopSectorId(m_sector_Pos, m_id);
        //Dir�� �������� ������ ���Ͱ� ���� ���̹Ƿ� ���͸� ��ĭ �̵��Ѵ�.
        SectorMove(m_gameinfo.m_cur_direction);
        sectors.PushSectorId(m_sector_Pos, m_id);
    }

    std::unordered_set<int> cur_sector = sectors.GetCurrentSector(m_sector_Pos);
    //std::cout << "cur_sector�� �����ϴ� ��ü�� ���� => " << cur_sector.size() << std::endl;
    // ���� ���� �������� ���̴� ��ü�� �����ϵ��� �Ѵ�.
    for (auto& c_id : cur_sector) { // ���̴� ��ü ����
        if (infos[c_id]->m_cur_state != STATE::ST_INGAME) continue;
        if (infos[c_id]->m_id == m_id) continue;
        if (InViewRange(infos[c_id]->m_id)) {
            new_View.insert(infos[c_id]->m_id);
        }
    }

    for (auto& c_id : new_View) { // Player �߰� �۾�
        if (infos[c_id]->m_cur_state != STATE::ST_INGAME) continue;
        if (infos[c_id]->m_id == m_id) continue;

        infos[c_id]->m_mtxView.lock();
        if (infos[c_id]->m_viewList.count(m_id)) { // ������ View�� �����Ҷ�
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
            // old_View �� ���� ������ ���� ���°� �Ǿ� ���� �� ����.
            // ������ ���Ḧ �ߴµ� view�� ���� �ְ� �Ǹ� �ȵ�.
            Send_remove_player(c_id);
            continue;
        }
        if (infos[c_id]->m_id == m_id) continue;

        if (new_View.count(c_id) == 0) { // �ֽ��� new_View�� old_View�� ��Ұ� ������� 
            Send_remove_player(c_id);
            infos[c_id]->Send_remove_player(m_id);
        }
    }
}

void ClientInfo::Send_add_object(const int& c_id, const int& visual)
{
    m_mtxView.lock();
    if (m_viewList.count(c_id) == 0) {
        m_viewList.insert(c_id); // unordered_set�� �̿��ϹǷ� ���Ҵ� ������.
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
    sc_p.x = cl_infos[c_id]->m_gameinfo.m_pos.x;
    sc_p.y = cl_infos[c_id]->m_gameinfo.m_pos.y;

    Send(&sc_p);
    //std::cout << "[" << m_id << "] ���� Send Add [" << c_id << "] Pos(" << sc_p.x << "," << sc_p.y << ")" << std::endl;
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


void ClientInfo::Move(char dir)
{
    Object::Move(dir);
}

