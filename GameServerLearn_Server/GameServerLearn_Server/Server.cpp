#include "Server.h"
#include "ClientInfo.h"

Server::Server()
{
	std::wcout.imbue(std::locale("korean"));
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 0), &wsa_data);
	m_ListenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
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

	sockaddr c_addr;
	int addr_size = static_cast<int>(sizeof(c_addr));
	SOCKET client_sock = WSAAccept(m_ListenSock, &c_addr, &addr_size, nullptr, 0);

	// Ŭ���̾�Ʈ ������ ������ ��� ���� sockaddr_in ����ü ����
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	// getpeername �Լ��� ����Ͽ� Ŭ���̾�Ʈ�� IP �ּ� ��������
	if (getpeername(client_sock, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen) == SOCKET_ERROR) {
		std::cerr << "getpeername failed: " << WSAGetLastError() << std::endl;
		// ���� ó��...
		closesocket(client_sock);
		// Listen ������ �ݴ� �ڵ�...
		assert(0);
	}
	// sockaddr_in ����ü���� IP �ּ� ��������
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);

	// IP �ּ� ���
	std::cout << "IP address: [" << ip << " ] Ŭ���̾�Ʈ�� ����Ǿ����ϴ�." <<std::endl;
	//ClientInfo 
	m_client_Infos[m_iClient_count++] = ClientInfo(client_sock, m_iClient_count); // Ŭ���̾�Ʈ ������ �����Ѵ�.
}

Server::~Server()
{
	closesocket(m_ListenSock);
	WSACleanup();
}

void Server::Logic()
{
	for (auto& [id, info] : m_client_Infos) {
		if (!Recv(info)) {
			if (m_client_Infos.size() == 0) { // 1���÷��̸� ���� �ӽù����̹Ƿ� ���߿� �� ����
				break;
			}
			continue;
		}
		//cout << info.GetId() << " �� Ŭ���̾�Ʈ�� ���� Recv �Ϸ�" << endl;
		AssembleDataFromBytes(info);
		ResetSendList();
		PackDataToBytes(info);
		if (!Send(info)) {
			continue;
		}
		//cout << id << " �� Ŭ���̾�Ʈ�� ���� Send �Ϸ�" << endl;
	}
}

void Server::ClientExit(int id)
{
	m_client_Infos[id].Exit();
	m_client_Infos.erase(id);
	cout << id << "�� Ŭ���̾�Ʈ�� ������ �����߽��ϴ�." << endl;
}

int Server::Send(ClientInfo& info)
{
	CHAR buf[BUFSIZE];
	memcpy(buf, m_SendReserveList.data(), m_SendReserveList.size());

	wsabuf[0].buf = buf;
	wsabuf[0].len = static_cast<ULONG>(m_SendReserveList.size());
	//cout << wsabuf[0].len << " ����Ʈ�� ������." << endl;

	DWORD sent_size;
	int res = WSASend(info.GetSock(), wsabuf, 1, &sent_size, 0, nullptr, nullptr);
	if (res != 0) {
		error_display("WSASend Error : ", WSAGetLastError());
		assert(0);
	}
	return 1;
}

int Server::Recv(ClientInfo& info)
{
	CHAR buf[BUFSIZE];

	wsabuf[0].buf = buf;
	wsabuf[0].len = BUFSIZE; // ������ ������ �������϶� �� ���� ���� �� ����.
	DWORD recv_size;
	DWORD recv_flag = 0;
	int res = WSARecv(info.GetSock(), wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr);
	//cout << "���� ������ => " << recv_size << endl;
	wsabuf[0].len = recv_size;
	if (res != 0) {
		error_display("WSARecv Error : ", WSAGetLastError());
		assert(0);
	}
	if (recv_size == 0) {
		ClientExit(info.GetId());
		return 0;
	}

	return 1;
}

void WriteToBuffer(vector<BYTE>& buffer,void* data, size_t size)
{
	for (int i = 0;i < size;++i) {
		buffer.push_back(((BYTE*)data)[i]);
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

void Server::AssembleDataFromBytes(ClientInfo& info)
{
	m_CmdList.insert(m_CmdList.begin(), wsabuf[0].buf, wsabuf[0].buf + wsabuf[0].len);

	while (m_CmdList.size() != 0)
	{
		BYTE cmd = *m_CmdList.begin();
		m_CmdList.erase(m_CmdList.begin());

		switch ((GameCommandList)cmd)
		{
		case GameCommandList::MOVE:
			cmd = *m_CmdList.begin();
			m_CmdList.erase(m_CmdList.begin());

			switch ((PlayerMoveDir)cmd)
			{
			case PlayerMoveDir::LEFT:
				info.PlayerMove((PlayerMoveDir)cmd);
				break;
			case PlayerMoveDir::RIGHT:
				info.PlayerMove((PlayerMoveDir)cmd);
				break;
			case PlayerMoveDir::UP:
				info.PlayerMove((PlayerMoveDir)cmd);
				break;
			case PlayerMoveDir::DOWN:
				info.PlayerMove((PlayerMoveDir)cmd);
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
}

void Server::PackDataToBytes(ClientInfo& info)
{
	m_client_Infos[info.GetId()].WriteData(m_SendReserveList);
	//cout << "m_SendReserveList�� ������ " << m_SendReserveList.size() << endl;
}
