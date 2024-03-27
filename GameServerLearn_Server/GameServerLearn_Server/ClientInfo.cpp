#include "Server.h"
#include "ClientInfo.h"
#include "DataBase.h"

extern void WriteToBuffer(vector<BYTE>& buffer, void* data, size_t size);

ClientInfo::ClientInfo()
{
	playerinfo.pos = { 0,0 };
}

ClientInfo::ClientInfo(SOCKET s, int i) : ClientInfo()
{
	sock = s;
	id = i;
}

ClientInfo::~ClientInfo()
{
}

SOCKET ClientInfo::GetSock()
{
	return sock;
}

int ClientInfo::GetId()
{
	return id;
}

void ClientInfo::Exit()
{
	closesocket(sock);
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

void ClientInfo::WriteData(vector<BYTE>& buffer)
{
    WriteToBuffer(buffer, &playerinfo, sizeof(PlayerInfo));
}

