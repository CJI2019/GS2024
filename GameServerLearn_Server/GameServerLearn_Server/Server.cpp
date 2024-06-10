#pragma once
#include "stdafx.h"

#include "Server.h"
#include "ClientInfo.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SOCKET Server::m_Server_sock;
SOCKET Server::m_cSock;
OVER_ALLOC Server::m_over;
HANDLE Server::m_hiocp;

std::array<ClientInfo, MAX_USER> Server::m_aClientInfos;

Server::Server()
{
	Init();
}

Server::~Server()
{
	//closesocket(m_Server_sock);
	//WSACleanup();
}

void Server::Init()
{
	std::wcout.imbue(std::locale("korean"));
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	m_Server_sock = WSASocket(AF_INET, SOCK_STREAM,0, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN addr_s;
	memset(&addr_s, 0, sizeof(addr_s));
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(PORT_NUM);
	addr_s.sin_addr.S_un.S_addr = INADDR_ANY;

	int res = bind(m_Server_sock, reinterpret_cast<sockaddr*>(&addr_s), sizeof(addr_s));
	if (res != 0) {
		error_display("WSASend Error : ", WSAGetLastError());
		assert(0);
	}
	res = listen(m_Server_sock, SOMAXCONN);
	if (res != 0) {
		error_display("WSASend Error : ", WSAGetLastError());
		assert(0);
	}

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	m_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_Server_sock), m_hiocp, 9999, 0);
	m_cSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_over.io_type = IO_TYPE::IO_ACCEPT;
	AcceptEx(m_Server_sock, m_cSock, m_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &m_over.over);

}

void Server::Accept()
{
	std::vector<std::thread> workerThreads;

	int nthread = std::thread::hardware_concurrency();
	for (int i = 0; i < nthread; ++i) {
		workerThreads.emplace_back(Worker_Thread);
	}

	for (auto& th : workerThreads) {
		th.join();
	}

	closesocket(m_Server_sock);
	WSACleanup();

	return;
}

void Server::Worker_Thread( )
{
	DWORD recvByte;
	ULONG_PTR key;
	WSAOVERLAPPED* over = nullptr;

	while (true) {
		OVER_ALLOC* o_alloc = nullptr;
		BOOL ret = GetQueuedCompletionStatus(m_hiocp, &recvByte, &key, &over, INFINITE);
		o_alloc = reinterpret_cast<OVER_ALLOC*>(over);

		if (ret == false) {
			if (o_alloc->io_type == IO_TYPE::IO_ACCEPT) {
				std::cout << "Accept Error";
			}
			else {
				std::cout << "GQCS Error on client[" << key << "]\n";
				Disconnect(static_cast<int>(key));
				if (o_alloc->io_type == IO_TYPE::IO_SEND) {
					delete o_alloc;
				}
				continue;
			}
		}

		if (recvByte == 0 && 
			(o_alloc->io_type == IO_TYPE::IO_RECV || o_alloc->io_type == IO_TYPE::IO_SEND))
		{
			Disconnect(static_cast<int>(key));
			if (o_alloc->io_type == IO_TYPE::IO_SEND) {
				delete o_alloc;
			}
			continue;
		}

		switch (o_alloc->io_type) {
		case IO_TYPE::IO_ACCEPT: {
			Accept_Logic(o_alloc);
			break;
		}
		case IO_TYPE::IO_RECV: {
			int remain_data = recvByte + m_aClientInfos[key].m_prev_remain_byte;
			char* p = reinterpret_cast<CHAR*>(o_alloc->send_buf);
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					m_aClientInfos[key].ProcessPacket(p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			m_aClientInfos[key].m_prev_remain_byte = remain_data;
			if (remain_data > 0) {
				memcpy(o_alloc->send_buf, p, remain_data);
			}
			m_aClientInfos[key].Recv();
			break;
		}
		case IO_TYPE::IO_SEND:
			delete o_alloc;
			break;
		default:
			break;
		}
	}
}

int Server::Get_new_clientId()
{
	int new_Id = -1;
	for (int i = 0; i < MAX_USER;++i) {
		if (m_aClientInfos[i].m_cur_state == STATE::ST_INGAME) continue;
		if (m_aClientInfos[i].m_cur_state == STATE::ST_ALLOC) continue;
		new_Id = i;
		break;
	}
	return new_Id;
}

void Server::Accept_Logic(OVER_ALLOC* o_alloc)
{
	int client_Id = Get_new_clientId();
	if (client_Id != -1) {
		{// lock_guard로 하는게 그냥 m_aClientInfos[client_Id].m_mtxlock.lock 이랑 무슨차이지?
			std::lock_guard<std::mutex> mtx(m_aClientInfos[client_Id].m_mtxlock);
			m_aClientInfos[client_Id].m_cur_state = STATE::ST_ALLOC;
		}

		m_aClientInfos[client_Id].m_id = client_Id;
		m_aClientInfos[client_Id].m_name[0] = 0;
		m_aClientInfos[client_Id].m_prev_remain_byte = 0;
		m_aClientInfos[client_Id].m_sock = m_cSock;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_cSock),
			m_hiocp, client_Id, 0);
		m_aClientInfos[client_Id].Recv();
		m_cSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	}
	else {
		std::cout << "Full of users.\n";
	}

	ZeroMemory(&m_over.over, sizeof(m_over.over));
	int addr_size = sizeof(SOCKADDR_IN);
	AcceptEx(m_Server_sock, m_cSock, m_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &m_over.over);

	// 클라이언트 주소 출력
	sockaddr_in* localSockaddr = NULL;
	sockaddr_in* remoteSockaddr = NULL;
	int localSockaddrLen = 0;
	int remoteSockaddrLen = 0;

	GetAcceptExSockaddrs(
		o_alloc->send_buf,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		(sockaddr**)&localSockaddr,
		&localSockaddrLen,
		(sockaddr**)&remoteSockaddr,
		&remoteSockaddrLen
	);

	char remoteAddress[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &remoteSockaddr->sin_addr, remoteAddress, INET_ADDRSTRLEN);
	int remotePort = ntohs(remoteSockaddr->sin_port);

	std::cout << "[" << client_Id << "]" << "Client IP: " << remoteAddress << ", Port: " << remotePort << std::endl;

}

void Server::Disconnect(int c_id)
{
	using namespace std;

	for (auto& pl : m_aClientInfos) { // 다른 플레이어에게 c_id 플레이어를 삭제를 알림.
		{
			lock_guard<mutex> ll(pl.m_mtxlock);
			if (pl.m_cur_state != STATE::ST_INGAME) continue;
		}
		if (pl.m_id == c_id) continue;
	}
	closesocket(m_aClientInfos[c_id].m_sock);

	lock_guard<mutex> ll(m_aClientInfos[c_id].m_mtxlock);
	m_aClientInfos[c_id].m_cur_state = STATE::ST_FREE;
}



void Server::ClientExit(LPWSAOVERLAPPED wsaover)
{
	//int id = m_umClientInfos[wsaover].playerinfo.id;

	//m_playerinfos[id] = nullptr;

	//m_umClientInfos.erase(wsaover);

	//auto p = find(m_playerinfos.begin(), m_playerinfos.end(), [id](const PlayerInfo& a) {
	//	return a.id == id;
	//	});

	//if (p != m_playerinfos.end()) {
	//	m_playerinfos[id] = nullptr;
	//	//m_playerinfos.erase(p);
	//}

	//std::cout << id << "번 클라이언트가 연결을 종료했습니다." << std::endl;
}

//버터에 데이터를 바이트 형태로 넣는다.(send할 데이터를 담을것임)
void Server::WriteToBuffer(std::vector<BYTE>& buffer,void* data, size_t size)
{
	for (int i = 0;i < size;++i) {
		buffer.push_back(((BYTE*)data)[i]);
	}
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

// [0] [1] [2]
void Server::WriteServerBuffer(std::vector<BYTE>& buffer, PlayerInfo* playerinfo)
{
	//m_playerinfos[playerinfo->id] = playerinfo;

	BYTE clientCount{};
	for (auto& info : m_playerinfos) {
		if(!info) continue;
		WriteToBuffer(buffer, info, sizeof(PlayerInfo));
		clientCount++;
	}
	buffer.insert(buffer.begin(), clientCount);
}