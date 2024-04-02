#include "Server.h"
#include "Player.h"

const char* IP_ADDRESS = "127.0.0.1";

Server::Server()
{ 
	//AllocConsole();
	//// 새로운 콘솔 창이 생성되었으므로 표준 입력/출력을 콘솔로 변경합니다.
	//freopen("CONIN$", "r", stdin);
	//freopen("CONOUT$", "w", stdout);

	//cout << "Server IP를 입력해주세요 : ";
	//char IP_char[256];
	//cin >> IP_char;

	std::wcout.imbue(std::locale("korean"));
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	m_ServerSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
	SOCKADDR_IN addr_s;
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, IP_ADDRESS, &addr_s.sin_addr);

	int res = connect(m_ServerSock, reinterpret_cast<sockaddr*>(&addr_s), sizeof(addr_s));
	if (res != 0) {
		error_display("WSARecv Error : ", WSAGetLastError());
		assert(0);
	}
}

Server::~Server()
{
	closesocket(m_ServerSock);
	WSACleanup();
}

void Server::Logic()
{
	Send();
	Recv();
}

void Server::Send()
{
	if (m_SendReserveList.size() == 0) {
		GameCommandList cmd = GameCommandList::NONE; // 보낼데이터가 존재하지 않더라도 데이터를 보냄.
		SendReserve(&cmd, sizeof(BYTE));
	}
	CHAR buf[BUFSIZE];
	memcpy(buf, m_SendReserveList.data(), m_SendReserveList.size());
	
	wsabuf[0].buf = buf;
	wsabuf[0].len = static_cast<ULONG>(m_SendReserveList.size());
	DWORD send_size;
	int res = WSASend(m_ServerSock, wsabuf, 1, &send_size, 0, nullptr, nullptr);
	if (res != 0) {
		error_display("WSASend Error : ", WSAGetLastError());
		exit(0);
		//assert(0);
	}
	ResetSendList();
}

void Server::Recv()
{
	wsabuf[0].buf = recvBuf;
	wsabuf[0].len = BUFSIZE; // 데이터 사이즈 여러개일때 한 번에 받을 수 있음.
	DWORD recv_size;
	DWORD recv_flag = 0;
	int res = WSARecv(m_ServerSock, wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr);
	if (res != 0) {
		error_display("WSARecv Error : ", WSAGetLastError());
		exit(0);
		//assert(0);
	}
}

vector<BYTE> Server::GetRecvBuffer()
{
	// 저장할 벡터 생성
	std::vector<BYTE> buffer;

	// 데이터 복사
	buffer.insert(buffer.begin(), recvBuf, recvBuf + wsabuf[0].len);
	return buffer;
}

void Server::SendReserve(void* data, size_t size)
{
	for (int i = 0;i < size;++i) {
		m_SendReserveList.push_back(((BYTE*)data)[i]);
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
