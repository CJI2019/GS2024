#include "Server.h"
#include "ClientInfo.h"
#include "DataBase.h"

extern void CALLBACK recv_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
extern void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

int g_id = 0;

ClientInfo::ClientInfo()
{
    playerinfo.id = g_id++;
	playerinfo.pos = { 0,0 };

    m_bUpdated = false;
}

ClientInfo::ClientInfo(SOCKET s, WSAOVERLAPPED* over) : ClientInfo()
{
    s_id = playerinfo.id;
	sock = s;
    m_WSAover = over;
    wsabuf[0].buf = buf;
    wsabuf[0].len = BUFSIZE;
}

ClientInfo::~ClientInfo()
{
    closesocket(sock);
    delete m_WSAover;
}

SOCKET ClientInfo::GetSock()
{
	return sock;
}

LPWSAOVERLAPPED ClientInfo::GetWSAoverlapped()
{
	return m_WSAover;
}

void ClientInfo::Recv()
{
    DWORD recv_flag = 0;
    ZeroMemory(m_WSAover, sizeof(*m_WSAover));
    int res = WSARecv(sock, wsabuf, 1, nullptr, &recv_flag, m_WSAover, recv_callback);
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

void ClientInfo::Send()
{
    memcpy(buf, m_SendReserveList.data(), m_SendReserveList.size());

    auto b = new EXP_OVER(buf, m_SendReserveList.size());
    int res = WSASend(sock, b->wsabuf, 1, nullptr, 0, &b->wsaover, send_callback);
    /*if (res != 0) {
        auto err_no = WSAGetLastError();
        if (WSA_IO_PENDING != err_no) {
            Server::error_display("WSASend Error : ", WSAGetLastError());
            assert(0);
        }
    }*/
}

void ClientInfo::Exit()
{
    serverFramework.ClientExit(m_WSAover);
}

void ClientInfo::PlayerMove(PlayerMoveDir dir)
{
    switch (dir)
    {
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

void ClientInfo::UpdateData(DWORD transfer_size)
{
    m_bUpdated = false;

    vector<BYTE> cmdList;
    cmdList.insert(cmdList.begin(), wsabuf[0].buf, wsabuf[0].buf + transfer_size);

    while (cmdList.size() != 0)
    {
        BYTE cmd = *cmdList.begin();
        cmdList.erase(cmdList.begin());

        switch ((GameCommandList)cmd)
        {
        case GameCommandList::MOVE:
            cmd = *cmdList.begin();
            cmdList.erase(cmdList.begin());

            switch ((PlayerMoveDir)cmd)
            {
            case PlayerMoveDir::LEFT:
                PlayerMove((PlayerMoveDir)cmd);
                break;
            case PlayerMoveDir::RIGHT:
                PlayerMove((PlayerMoveDir)cmd);
                break;
            case PlayerMoveDir::UP:
                PlayerMove((PlayerMoveDir)cmd);
                break;
            case PlayerMoveDir::DOWN:
                PlayerMove((PlayerMoveDir)cmd);
                break;
            default:
                assert(0);
            }
            break;
        case GameCommandList::NONE:
        default:
            break;
        }
    }

    m_bUpdated = true;
}

