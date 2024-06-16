#pragma once
#include "DataBase.h"
#include "ClientInfo.h"

#define serverFramework Server::GetInstance()


class Timer_Event {
public:
	Timer_Event() {}
	~Timer_Event() {}

public:
	std::chrono::system_clock::time_point m_start_Time;
	TIMER_EVENT_TYPE m_event_type;
	int m_object_id;
public:
	bool operator<(const Timer_Event& other) const {
		return m_start_Time > other.m_start_Time;
	}
};

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
	static void Worker_Thread();
	static void error_display(const char* msg, int err_no);

	int Get_new_clientId();
	void Accept_Logic(OVERLAPPED_EX* o_alloc);

	void Init();
	void Accept();
	void Disconnect(int c_id);

	HANDLE& GetHandle() { return m_hiocp; }

protected:
	SOCKET m_Server_sock;
	SOCKET m_cSock;
	OVERLAPPED_EX m_over;
	HANDLE m_hiocp;

	concurrent_priority_queue<Timer_Event> m_timerQueue;

	array<shared_ptr<Object>, MAX_USER + MAX_NPC> m_aObjectInfos;

	UINT m_iClient_count = 0; //접속한 개수가 아닌 접속했던 클라의 개수

	Sector m_Sector;
public:
	//Timer_Queue
	concurrent_priority_queue<Timer_Event>& GetTimerQueue() { return m_timerQueue; }
	void PushTimer(TIMER_EVENT_TYPE event_type, int object_id);

	array<shared_ptr<Object>, MAX_USER + MAX_NPC>& GetObjectInfos() { return m_aObjectInfos; }

	Sector& GetSector() { return m_Sector; }
};
