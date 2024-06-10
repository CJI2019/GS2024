#pragma once
#include "stdafx.h"

#include "Server.h"
#include "ClientInfo.h"
#include "DataBase.h"

extern void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
extern void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(0, ROW_X - 1);

int g_id = 0;

ClientInfo::ClientInfo()
{
    m_id = -1;

	playerinfo.pos = { 0,0 };

    m_bUpdated = false;

    m_cur_state = STATE::ST_FREE;

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
    auto b = new OVER_ALLOC(reinterpret_cast<char*>(packet));
    int res = WSASend(m_sock, &b->m_wsabuf, 1, nullptr, 0, &b->over, 0);
   // std::cout << "송신\n";
    /*if (res != 0) {
        auto err_no = WSAGetLastError();
        if (WSA_IO_PENDING != err_no) {
            Server::error_display("WSASend Error : ", WSAGetLastError());
            assert(0);
        }
    }*/
}

void ClientInfo::ProcessPacket(char* packet)
{
    switch (packet[2])
    {
    case CS_LOGIN: {
        std::cout << "[" << m_id << "] 로그인\n";
        std::array<ClientInfo, MAX_USER>& infos = serverFramework.GetClientInfo();
        infos[m_id].Send_login();
        {
            std::lock_guard<std::mutex> ll(m_mtxlock);
            m_cur_state = STATE::ST_INGAME;
        }

        for (auto& cl : infos) {
            {
                std::lock_guard<std::mutex> ll(cl.m_mtxlock);
                if (cl.m_cur_state != STATE::ST_INGAME) continue;
            }
            if (cl.m_id == m_id) continue;

           // cl.m_mtxlock.lock();
            cl.Send_add_player(m_id);
           // cl.m_mtxlock.unlock();

            //infos[m_id].m_mtxlock.lock();
            infos[m_id].Send_add_player(cl.m_id);
            //std::cout << "[" << m_id << "] 에게 " << "Add Player Send\n";
            //infos[m_id].m_mtxlock.unlock();
		}

        break;
    }
    case CS_MOVE: {
        move_count++;
        CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

        //m_mtxlock.lock();
        m_LastMoveTime = p->move_time;
        PlayerMove(p->direction);
        Send_move_player(p);
        
        //m_mtxlock.unlock();

        //std::cout << "[" << m_id << "] MOVE\n";
        //std::cout << move_count << "번 움직임\n";
        //std::cout << playerinfo.pos.x << "," << playerinfo.pos.y << std::endl;
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

    playerinfo.pos.x = dis(gen);
    playerinfo.pos.y = dis(gen);

    sc_p.x = playerinfo.pos.x;
    sc_p.y = playerinfo.pos.y;
    sc_p.size = sizeof(SC_LOGIN_INFO_PACKET);

    Send(&sc_p);
    std::cout << "Send 로그인\n";
}

void ClientInfo::Send_move_player(void* packet) // 굳이 매개변수를 받을 필요는 현재까지는 없음.
{
    SC_MOVE_OBJECT_PACKET sc_p;

    sc_p.type = SC_MOVE_OBJECT;
    sc_p.id = m_id;
    sc_p.x = playerinfo.pos.x;
    sc_p.y = playerinfo.pos.y;
    sc_p.size = sizeof(SC_MOVE_OBJECT_PACKET);
    sc_p.move_time = m_LastMoveTime;

    Send(&sc_p);

    auto& infos = serverFramework.GetClientInfo();
    for (auto& info : infos) {
        if (info.m_cur_state != STATE::ST_INGAME) continue;
        if (info.m_id == m_id) continue;
        info.Send(&sc_p);
    }
}

void ClientInfo::Send_add_player(int c_id)
{
    SC_ADD_OBJECT_PACKET sc_p;

    sc_p.type = SC_ADD_OBJECT;
    sc_p.id = c_id;
    sc_p.size = sizeof(SC_ADD_OBJECT_PACKET);

    std::array<ClientInfo, MAX_USER>& cl_infos = serverFramework.GetClientInfo();
    sc_p.x = cl_infos[c_id].playerinfo.pos.x;
    sc_p.y = cl_infos[c_id].playerinfo.pos.y;

    Send(&sc_p);
    //std::cout << "[" << m_id << "] 에게 Send Add [" << c_id << "] Pos(" << sc_p.x << "," << sc_p.y << ")" << std::endl;
    
}

void ClientInfo::PlayerMove(char dir)
{
    switch (static_cast<PlayerMoveDir>(dir))
    {
    /*case PlayerMoveDir::NONE:
        break;*/
    case PlayerMoveDir::LEFT:
        if (0 < playerinfo.pos.x) {
            playerinfo.pos.x -= 1;
        }
        //cout << "왼쪽" << endl;
        break;
    case PlayerMoveDir::RIGHT:
        if (playerinfo.pos.x < ROW_X - 1) {
            playerinfo.pos.x += 1;
        }
        //cout << "오른쪽" << endl;
        break;
    case PlayerMoveDir::UP:
        if (0 < playerinfo.pos.y) {
            playerinfo.pos.y -= 1;
        }
        //cout << "위" << endl;
        break;
    case PlayerMoveDir::DOWN:
        if (playerinfo.pos.y < COL_Y - 1) {
            playerinfo.pos.y += 1;
        }
        //cout << "아래" << endl;
        break;
    default:
        assert(0);
    }
}

void ClientInfo::WriteData()
{
    m_SendReserveList.clear();
    serverFramework.WriteServerBuffer(m_SendReserveList, &playerinfo);

}
