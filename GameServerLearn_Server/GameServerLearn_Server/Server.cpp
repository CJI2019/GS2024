#include "Server.h"
#include "ClientInfo.h"

void CALLBACK send_callback(DWORD err_res,
	DWORD transfer_size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	auto b = reinterpret_cast<EXP_OVER*>(p_wsaover);
	delete b;
}

void CALLBACK recv_callback(DWORD err_res,
	DWORD transfer_size,
	LPWSAOVERLAPPED p_wsaover,
	DWORD rec_flag)
{
	auto& infos = serverFramework.GetClientInfos();
	infos[p_wsaover].UpdateData(transfer_size);
	infos[p_wsaover].WriteData();
	infos[p_wsaover].Send();
	infos[p_wsaover].Recv();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Server::Server()
{
	Init();
}

Server::~Server()
{
	closesocket(m_ListenSock);
	WSACleanup();
}
	
void Server::Init()
{
	std::wcout.imbue(std::locale("korean"));
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	m_ListenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN addr_s;
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(SERVER_PORT);
	addr_s.sin_addr.s_addr = htonl(ADDR_ANY);

	int res = bind(m_ListenSock, reinterpret_cast<sockaddr*>(&addr_s), sizeof(addr_s));
	if (res != 0) {
		error_display("WSASend Error : ", WSAGetLastError());
		assert(0);
	}
	res = listen(m_ListenSock, SOMAXCONN);
	if (res != 0) {
		error_display("WSASend Error : ", WSAGetLastError());
		assert(0);
	}
}

void Server::Accept()
{
	sockaddr c_addr;
	int addr_size = static_cast<int>(sizeof(c_addr));
	SOCKET client_sock = WSAAccept(m_ListenSock, &c_addr, &addr_size, nullptr, 0);
	LPWSAOVERLAPPED p_over = new WSAOVERLAPPED;
	//ClientInfo 
	m_umClientInfos.try_emplace(p_over, client_sock, p_over); // 클라이언트 정보를 저장한다.
	PlayerInfo* pinfo = new PlayerInfo{ 0,{0,0} };
	m_playerinfos.push_back(pinfo);
	m_umClientInfos[p_over].Recv();

	// 클라이언트 소켓의 정보를 얻기 위한 sockaddr_in 구조체 생성
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	// getpeername 함수를 사용하여 클라이언트의 IP 주소 가져오기
	if (getpeername(client_sock, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen) == SOCKET_ERROR) {
		std::cerr << "getpeername failed: " << WSAGetLastError() << std::endl;
		// 오류 처리...
		closesocket(client_sock);
		assert(0); // 강제종료
	}
	// sockaddr_in 구조체에서 IP 주소 가져오기
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
	// IP 주소 출력
	std::cout << "IP address: [" << ip << " ] 클라이언트가 연결되었습니다." << std::endl;
}

void Server::ClientExit(LPWSAOVERLAPPED wsaover)
{
	int id = m_umClientInfos[wsaover].playerinfo.id;

	m_playerinfos[id] = nullptr;

	m_umClientInfos.erase(wsaover);

	//auto p = find(m_playerinfos.begin(), m_playerinfos.end(), [id](const PlayerInfo& a) {
	//	return a.id == id;
	//	});

	//if (p != m_playerinfos.end()) {
	//	m_playerinfos[id] = nullptr;
	//	//m_playerinfos.erase(p);
	//}

	cout << id << "번 클라이언트가 연결을 종료했습니다." << endl;
}

//버터에 데이터를 바이트 형태로 넣는다.(send할 데이터를 담을것임)
void Server::WriteToBuffer(vector<BYTE>& buffer,void* data, size_t size)
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
void Server::WriteServerBuffer(vector<BYTE>& buffer, PlayerInfo* playerinfo)
{
	m_playerinfos[playerinfo->id] = playerinfo;

	BYTE clientCount{};
	for (auto& info : m_playerinfos) {
		if(!info) continue;
		WriteToBuffer(buffer, info, sizeof(PlayerInfo));
		clientCount++;
	}
	buffer.insert(buffer.begin(), clientCount);

}