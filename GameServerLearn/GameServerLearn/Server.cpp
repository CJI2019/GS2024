#pragma once
#include "stdafx.h"

#include "Server.h"
#include "Player.h"

#include <io.h>
#include <fcntl.h>

const char* IP_ADDRESS = "127.0.0.1";

void RedirectIOToConsole() {
	// 새로운 콘솔 창을 할당합니다.
	AllocConsole();

	// 파일 디스크립터를 콘솔 입출력에 연결합니다.
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);

	// 콘솔 입출력 스트림을 C++ 표준 스트림과 연결합니다.
	std::ios::sync_with_stdio();
}

Server::Server()
{ 
	RedirectIOToConsole();
	std::wcout.imbue(std::locale("korean"));
	//RedirectIOToConsole();
	cout << "Server IP를 입력해주세요 : ";
	char IP_char[256];
	std::cin >> IP_char;
	char id_name[20];
	cout << "ID를 입력해주세요 : ";
	std::cin >> id_name;

	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	m_Sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	if (m_Sock == INVALID_SOCKET) {
		error_display("WSARecv Error : ", WSAGetLastError());
		WSACleanup();
		assert(0);
	}

	SOCKADDR_IN addr_s;
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(PORT_NUM);
	inet_pton(AF_INET, IP_char /*IP_ADDRESS*/, &addr_s.sin_addr);

	int res = connect(m_Sock, reinterpret_cast<sockaddr*>(&addr_s), sizeof(addr_s));
	if (res != 0) {
		error_display("WSARecv Error : ", WSAGetLastError());
		assert(0);
	}
	u_long mode = 1; //non-blocking mode
	ioctlsocket(m_Sock, FIONBIO, &mode);

	CS_LOGIN_PACKET packet;
	packet.type = CS_LOGIN;
	packet.size = sizeof(CS_LOGIN_PACKET);
	strncpy(packet.name, id_name, strlen(id_name) + 1);
	m_mainPlayerName = packet.name;
	SendReserve(&packet, sizeof(CS_LOGIN_PACKET));

	remain_recv_byte = 0;
}

Server::~Server()
{
	CS_LOGOUT_PACKET packet;
	packet.type = CS_LOGOUT;
	packet.size = sizeof(CS_LOGOUT_PACKET);

	SendReserve(&packet, sizeof(CS_LOGIN_PACKET));
	Send();

	closesocket(m_Sock);
	WSACleanup();
}

void Server::Logic()
{
	Send();
	Recv();
}

void Server::Send()
{
	CHAR buf[BUFSIZE];
	if (m_SendReserveList.size() == 0) {
		return;
	}
	memcpy(buf, m_SendReserveList.data(), m_SendReserveList.size());

	//over->SetSendPacket(buf);
	OVERLAPPED_EX* over = new OVERLAPPED_EX(buf);

	DWORD send_size;
	int res = WSASend(m_Sock, &over->m_wsabuf, 1, &send_size, 0, &over->over, nullptr);

	if (res != 0) {
		error_display("WSASend Error : ", WSAGetLastError());
		assert(0);
	}
	delete over;

	ResetSendList();
}

void Server::Recv()
{
	//OVER_ALLOC* m_over_ex = new OVER_ALLOC;

	//m_over_ex->RecvPrepare(packet_buf);
	m_over_ex.m_wsabuf.buf = packet_buf + remain_recv_byte;
	m_over_ex.m_wsabuf.len = BUFSIZE - remain_recv_byte;
	remain_recv_byte = 0;

	DWORD recv_size{};
	DWORD recv_flag = 0;
	int res = WSARecv(m_Sock, &m_over_ex.m_wsabuf, 1, &recv_size, &recv_flag, nullptr/*&m_over_ex->over*/, nullptr);

	if (res != 0) {
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING && error != WSAEWOULDBLOCK) {
			error_display("WSARecv Error : ", error);
			assert(0);
		}
		else if (error == WSA_IO_PENDING) {
			int x = 0;
		}
	}
	//delete m_over_ex;
}

vector<CHAR> Server::GetRecvBuffer()
{
	vector<CHAR> buffer;

	buffer.insert(buffer.begin(), packet_buf, packet_buf + BUFSIZE);
	return buffer;
}

void Server::InitBuffer(int interval)
{
	memset(packet_buf + interval, 0, BUFSIZE - interval);
}

void Server::SendReserve(void* data, size_t size)
{
	for (int i = 0;i < size;++i) {
		m_SendReserveList.push_back(((CHAR*)data)[i]);
	}
}

void Server::ResetSendList()
{
	m_SendReserveList.clear();
}



void Server::error_display(const char* msg, int err_no)
{
	WCHAR* msgbuf = nullptr;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		nullptr, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&msgbuf, 0, nullptr);
	std::cout << msg;
	std::wcout << msgbuf << std::endl;
	LocalFree(msgbuf);
}
