#define serverFramework Server::GetInstance()

//constexpr char SERVER_ADDR[] = "127.0.0.1";

class Server
{
private:
	Server();
public:
	static Server& GetInstance() {
		static Server instance; // 정적 변수로 인스턴스를 유지, 지연 초기화
		return instance;
	}
	~Server();

	// 서버에서 데이터를 주고받는 작업을 하는 함수
	void Logic();

	void Send();

	void Recv();
	vector<CHAR> GetRecvBuffer();
	void InitBuffer(int interval);

	void SendReserve(void* data, size_t size);
	void ResetSendList();

	void error_display(const char* msg, int err_no);

	//void Worker_Thread();
//protected:
	OVERLAPPED_EX m_over_ex;

	SOCKET m_Sock;
	WSABUF wsabuf[1];

	vector<CHAR> m_SendReserveList;

	CHAR packet_buf[BUFSIZE];
	int remain_recv_byte;

	std::string m_mainPlayerName;
};

