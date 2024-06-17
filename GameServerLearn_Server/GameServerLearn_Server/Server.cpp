#pragma once
#include "stdafx.h"
#include "Server.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Server::Server()
{
	DB; // DB 접속
	cout << "DB 접속 완료\n";
	Init();
	for (int i = 0; i < MAX_USER;++i) {
		m_aObjectInfos[i] = std::make_shared<ClientInfo>();
		m_aObjectInfos[i]->m_gameinfo.m_visual = visual_player;

		// 유저는 로그인 할때 스크립트 정보를 저장하도록 변경해야함.
		m_aObjectInfos[i]->m_cLua.SetScriptInfo(i); 
	}

	for (int i = MAX_USER; i < MAX_USER+MAX_NPC;++i) {
		m_aObjectInfos[i] = std::make_shared<NPC>();
		m_aObjectInfos[i]->m_id = i;
		m_aObjectInfos[i]->m_cur_state = STATE::ST_INGAME;
		m_aObjectInfos[i]->m_gameinfo.m_visual = rand() % 2 ? visual_NPC1 : visual_NPC2;

		m_aObjectInfos[i]->m_sector_Pos = m_Sector.AllocSectorId(m_aObjectInfos[i]->m_gameinfo.m_pos, m_aObjectInfos[i]->m_id);

		m_aObjectInfos[i]->m_cLua.SetScriptInfo(i);
		m_aObjectInfos[i]->m_name = m_aObjectInfos[i]->m_cLua.m_name;
	}

	/*DB_ID_INFO d_data;
	d_data.m_strUserid = "2019182042";
	DB.DB_Process(&d_data,LOGIN);
	cout << d_data.m_strUserid << " " << d_data.m_xPos << "," << d_data.m_yPos << endl;*/
	/*for (int i = MAX_USER; i < MAX_USER + MAX_NPC;++i) {
		PushTimer(TIMER_EVENT_TYPE::TE_NPC_RANDOM_MOVE, m_aObjectInfos[i]->m_id);
	}*/ 

	cout << "Object 초기화 완료\n" << endl;
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

void Server::Start()
{
	std::vector<std::thread> workerThreads;

	int nthread = std::thread::hardware_concurrency();
	for (int i = 0; i < nthread - 1; ++i) {
		workerThreads.emplace_back(Worker_Thread);
	}
	workerThreads.emplace_back(Timer_Thread);

	for (auto& th : workerThreads) {
		th.join();
	}

	closesocket(m_Server_sock);
	WSACleanup();

	return;
}

void Server::Timer_Thread()
{
	using namespace std;
	using namespace chrono;
	using enum TIMER_EVENT_TYPE;
	using enum IO_TYPE;

	auto& sf = serverFramework;
	auto& timerQueue = sf.GetTimerQueue();
	HANDLE& hiocp = sf.GetHandle();

	while (true) {
		Timer_Event t_event;

		if (!timerQueue.try_pop(t_event)) {
			this_thread::sleep_for(1ms);
			continue;
		}

		if (t_event.m_start_Time > system_clock::now()) {
			timerQueue.push(move(t_event));
			this_thread::sleep_for(1ms);
			continue;
		}

		OVERLAPPED_EX* over_ex = new OVERLAPPED_EX;
		switch (t_event.m_event_type) {
		case TE_NPC_RANDOM_MOVE: {
			/*if (t_event.m_object_id == 200000) {
				cout << system_clock::now() << endl;
			}*/
			over_ex->io_type = IO_NPC_MOVE;
			break;
		}
		case TE_ATTACK: {
			over_ex->io_type = IO_ATTACK;
			break;
		}
		default:
			break;
		}

		PostQueuedCompletionStatus(hiocp, 1, t_event.m_object_id, &over_ex->over);

	}
}

void Server::Worker_Thread( )
{
	using enum IO_TYPE;

	DWORD recvByte;
	ULONG_PTR key;
	WSAOVERLAPPED* over = nullptr;
	
	auto& sf = serverFramework;

	while (true) {
		OVERLAPPED_EX* o_alloc = nullptr;
		BOOL ret = GetQueuedCompletionStatus(sf.m_hiocp, &recvByte, &key, &over, INFINITE);
		o_alloc = reinterpret_cast<OVERLAPPED_EX*>(over);

		if (ret == false) {
			if (o_alloc->io_type == IO_ACCEPT) {
				cout << "Accept Error";
			}
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				sf.m_aObjectInfos[key]->Exit();
				//sf.Disconnect(static_cast<int>(key));
				if (o_alloc->io_type == IO_SEND) {
					delete o_alloc;
				}
				continue;
			}
		}

		if (recvByte == 0 && 
			(o_alloc->io_type == IO_RECV || o_alloc->io_type == IO_SEND))
		{
			cout << "zzz 종료\n";
			sf.m_aObjectInfos[key]->Exit();
			//sf.Disconnect(static_cast<int>(key));
			if (o_alloc->io_type == IO_SEND) {
				delete o_alloc;
			}
			continue;
		}

		switch (o_alloc->io_type) {
		case IO_ACCEPT: {
			sf.Accept_Logic(o_alloc);
			break;
		}
		case IO_RECV: {
			shared_ptr<ClientInfo> cl_info = static_pointer_cast<ClientInfo>(sf.m_aObjectInfos[key]);

			int remain_data = recvByte + cl_info->m_prev_remain_byte;
			char* p = reinterpret_cast<CHAR*>(o_alloc->send_buf);
			bool p_Fail = true;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					// Player인 것만 캐스팅 시켜서 사용하면 되는데..
					p_Fail = cl_info->ProcessPacket(p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else {
					break;
				}
			}
			cl_info->m_prev_remain_byte = remain_data;
			if (remain_data > 0) {
				memcpy(o_alloc->send_buf, p, remain_data);
			}
			if (p_Fail) {
				cl_info->Recv();
			}
			break;
		}
		case IO_SEND:
			delete o_alloc;
			break;
		case IO_NPC_MOVE:
			sf.m_aObjectInfos[key]->Move(-1); //NPC 의 랜덤무브
			//cout << key << "NPCMOVE\n";
			break;
		case IO_ATTACK:
			sf.m_aObjectInfos[key]->Attack();
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
		lock_guard<mutex> cl_il(m_aObjectInfos[i]->m_mtxlock);

		if (m_aObjectInfos[i]->m_cur_state == STATE::ST_INGAME) continue;
		if (m_aObjectInfos[i]->m_cur_state == STATE::ST_ALLOC) continue;
		new_Id = i;
		break;
	}
	return new_Id;
}

void Server::Accept_Logic(OVERLAPPED_EX* o_alloc)
{
	int client_Id = Get_new_clientId();

	if (client_Id != -1) {
		{
			std::lock_guard<std::mutex> mtx(m_aObjectInfos[client_Id]->m_mtxlock);
			m_aObjectInfos[client_Id]->m_cur_state = STATE::ST_ALLOC;
		}
		shared_ptr<ClientInfo> cl_info = static_pointer_cast<ClientInfo>(m_aObjectInfos[client_Id]);
		cl_info->m_id = client_Id;
		cl_info->m_name[0] = 0;
		cl_info->m_prev_remain_byte = 0;
		cl_info->m_sock = m_cSock;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_cSock),
			m_hiocp, client_Id, 0);
		cl_info->Recv();
		m_cSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	else {
		cout << "Full of users.\n";
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

	//cout << "[" << client_Id << "]" << "Client IP: " << remoteAddress << ", Port: " << remotePort << endl;

}

void Server::Disconnect(int c_id)
{
	m_aObjectInfos[c_id]->m_mtxView.lock();
	auto oldView = m_aObjectInfos[c_id]->m_viewList;
	m_aObjectInfos[c_id]->m_mtxView.unlock();

	for (auto& view_id : oldView) { // View에 있던 다른 플레이어에게 c_id 플레이어를 삭제를 알림.
		{
			lock_guard<mutex> ll(m_aObjectInfos[view_id]->m_mtxlock);
			if (m_aObjectInfos[view_id]->m_cur_state != STATE::ST_INGAME) continue;
		}
		if (m_aObjectInfos[view_id]->m_id == c_id) continue;

		m_aObjectInfos[view_id]->Send_remove_player(c_id);
	}
	closesocket(static_pointer_cast<ClientInfo>(m_aObjectInfos[c_id])->m_sock);

	lock_guard<mutex> ll(m_aObjectInfos[c_id]->m_mtxlock);
	m_aObjectInfos[c_id]->m_cur_state = STATE::ST_FREE;
	m_aObjectInfos[c_id]->m_viewList.clear();
	m_aObjectInfos[c_id]->m_id = -1;
}

void Server::PushTimer(TIMER_EVENT_TYPE event_type, int object_id, std::chrono::seconds sec)
{
	Timer_Event t_event;

	t_event.m_event_type = event_type;
	t_event.m_object_id = object_id;
	t_event.m_start_Time = std::chrono::system_clock::now() + sec;
	
	m_timerQueue.push(t_event);
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
