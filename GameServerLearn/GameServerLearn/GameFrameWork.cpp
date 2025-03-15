#pragma once
#include "stdafx.h"
#include "GameFrameWork.h"
#include "Server.h"

POINT** GameFrameWork::PanPosition;

extern UINT16 gRectHeightSize, gRectWidthSize;

GameFrameWork::GameFrameWork()
{
	if (!PanPosition) {
		PanPosition = new POINT * [ROW_X];
		for (int i = 0;i < COL_Y;++i) {
			PanPosition[i] = new POINT[COL_Y];
		}
	}
	m_Map = new CTile * [ROW_X];
	for (int i = 0;i < COL_Y;++i) {
		m_Map[i] = new CTile[COL_Y];
	}
	
	m_vSceneObject.reserve(MAX_USER + MAX_NPC);
	//m_vSceneObject.push_back(make_unique<Player>());
	for (int i = 0; i < MAX_USER;++i) {
		m_vSceneObject.emplace_back(make_shared<Player>());
	}
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC;++i) {
		m_vSceneObject.emplace_back(make_shared<NPC>());
	}

	m_objectCount = 0;
}

void GameFrameWork::InitMap()
{
	int viewSize = 20;
	int rectSize = gRectHeightSize / viewSize; // 체스판 한 칸의 크기
	int boardSize = rectSize * viewSize;       // 체스판 전체 크기

	int centerCustomXpos = (gRectWidthSize - gRectHeightSize) / 2;
	RECT rect;
	for (int y = 0; y < COL_Y; y++) {
		for (int x = 0; x < ROW_X; x++) {
			rect.left = (x * rectSize) + centerCustomXpos;
			rect.top = y * rectSize;
			rect.right = ((x + 1) * rectSize) + centerCustomXpos;
			rect.bottom = (y + 1) * rectSize;

			PanPosition[x][y] = { (rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2 };
			m_Map[x][y].x = (rect.left + rect.right) / 2;
			m_Map[x][y].y = (rect.top + rect.bottom) / 2;
			if (x == 0 && y == 0) {
				m_Map[x][y].color = RGB(0, 200, 200);
			}
			else {
				m_Map[x][y].color = rand() % 2 ? RGB(200, 200, 200) : RGB(100, 100, 100);
			}
		}
	}
}

GameFrameWork::~GameFrameWork()
{
	for (int i = 0;i < ROW_X;++i) {
		if (PanPosition[i]) {
			delete PanPosition[i];
		}
	}
	if (PanPosition) {
		delete[] PanPosition;
	}
}

void GameFrameWork::KeyInput(float elapsedTime)
{
	if (m_playerId == -1) {
		return;
	}

	static UCHAR pKeysBuffer[256];
	
	if (GetKeyboardState(pKeysBuffer)) {
		DWORD DIR = 0;

		if (pKeysBuffer[VK_UP] & 0xF0) DIR |= DIR_UP;
		if (pKeysBuffer[VK_DOWN] & 0xF0) DIR |= DIR_DOWN;
		if (pKeysBuffer[VK_LEFT] & 0xF0) DIR |= DIR_LEFT;
		if (pKeysBuffer[VK_RIGHT] & 0xF0) DIR |= DIR_RIGHT;
		if (pKeysBuffer['T'] & 0xF0) {
			static float cooltime = 0.f;
			if (cooltime < 0.f) {
				CS_TELEPORT_PACKET packet;
				packet.type = CS_TELEPORT;
				packet.size = sizeof(CS_TELEPORT_PACKET);
				serverFramework.SendReserve(&packet, packet.size);
				cooltime = 0.3f;
			}
			else {
				cooltime -= elapsedTime;
			}
		}

		m_vSceneObject[m_playerId]->Move(DIR, elapsedTime);
	}

}

void GameFrameWork::AddPlayerObject(void* buffer)
{
	m_objectCount++;
	SC_ADD_OBJECT_PACKET* packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(buffer);

	int newId = packet->id;
	m_vSceneObject[newId]->SetId(packet->id);
	m_vSceneObject[newId]->SetPosition(Vector2(packet->x, packet->y));
	m_vSceneObject[newId]->SetName(packet->name);
	m_vSceneObject[newId]->SetVisual(packet->visual);

	m_viewObjects.insert(newId);
}

void GameFrameWork::DelPlayerObject(void* buffer)
{
	m_objectCount--;
	SC_REMOVE_OBJECT_PACKET* packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(buffer);
	// 객체의 데이터를 초기화 시킨다.
	m_vSceneObject[packet->id]->SetId(-1);
	m_viewObjects.erase(packet->id);
	//m_vSceneObject.erase(m_vSceneObject.begin() + packet->id);
}

void GameFrameWork::Update(float elapsedTime)
{
	serverFramework.Logic();

	WriteData();

	KeyInput(elapsedTime);

	FindNearestObject(); 
	//현재 서버에서 공격 처리를 하므로, 가장 가까운 상대를 골라내는 작업만함.
	//  -> 이미 서버에서 골라냈는데 동일한 작업을 두번하므로 이는 빼고 서버에서 패킷을 받도록 바꿔야함.

	//auto Attack
	float m_attackCooltime = 0.0f;
	if (m_attackCooltime > ATTACKCOOLTIME) {
		// 쿨
	}
	m_attackCooltime += elapsedTime;
	if (m_nearest_obj_id != -1) { // 근접 npc를 공격한다.
		// 공격 패킷을 보낸다.
	}
}

void GameFrameWork::WriteData()
{
	auto buffer = serverFramework.GetRecvBuffer();

	unsigned short packet_size;

	while (buffer.size() != 0)
	{ // 패킷 사이즈가 2바이트이기 떄문에 이에 대한 처리 생각해야함.
		if (buffer.size() > 1) {
			memcpy(&packet_size, buffer.data(), sizeof(unsigned short));
			if (packet_size >= BUFSIZE) break;
			if (buffer.size() < packet_size) {
				serverFramework.remain_recv_byte += buffer.size();
				memcpy(serverFramework.packet_buf, buffer.data(), buffer.size());
				serverFramework.InitBuffer(buffer.size());
				break;
			} //[10] [3]가 남음. [3]에 해당하는 버퍼를 packetbuf의 앞으로 당겨와야함.
			else if(packet_size == 0){
				serverFramework.InitBuffer(0);
				break;
			}
		}
		else { // buffer의 사이즈가 1바이트 뿐인것임. 2바이트 중 1바이트만 현재 버퍼에 온것을 뜻함.
			//남은 1바이트를 처리
			serverFramework.remain_recv_byte += 1;
			memcpy(serverFramework.packet_buf, buffer.data(), buffer.size());
			serverFramework.InitBuffer(1);
			break;
		}
		switch (buffer[2]) // 패킷의 유형을 나타냄 1바이트로 표현 256개 이상의 패킷 유형을 만들지는 않을 것이므로..
		{
		case SC_LOGIN_INFO: {
			SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(buffer.data());
			m_playerId = packet->id; // 메인 클라이언트 Id Set
			m_vSceneObject[m_playerId]->SetId(packet->id);
			m_vSceneObject[m_playerId]->SetPosition(Vector2(packet->x, packet->y));
			m_vSceneObject[m_playerId]->SetName(serverFramework.m_mainPlayerName.c_str());
			m_vSceneObject[m_playerId]->m_hp = packet->hp;
			m_vSceneObject[m_playerId]->m_maxhp = packet->max_hp;
			m_vSceneObject[m_playerId]->m_level = packet->level;
			m_vSceneObject[m_playerId]->m_exp = packet->exp;
			m_viewObjects.insert(m_playerId);
			buffer.erase(buffer.begin(), buffer.begin() + sizeof(SC_LOGIN_INFO_PACKET));
			break;
		}
		case SC_MOVE_OBJECT: {
			SC_MOVE_OBJECT_PACKET* packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(buffer.data());
			m_vSceneObject[packet->id]->SetPosition(Vector2(packet->x, packet->y));
			buffer.erase(buffer.begin(), buffer.begin() + sizeof(SC_MOVE_OBJECT_PACKET));
			break;
		}
		case SC_ADD_OBJECT: {
			AddPlayerObject(buffer.data());
			buffer.erase(buffer.begin(), buffer.begin() + sizeof(SC_ADD_OBJECT_PACKET));
			break;
		}
		case SC_REMOVE_OBJECT: {
			DelPlayerObject(buffer.data());
			buffer.erase(buffer.begin(), buffer.begin() + sizeof(SC_REMOVE_OBJECT_PACKET));
			break;
		}
		case SC_CHAT: { // 전투 메시지 등 채팅 패킷
			SC_CHAT_PACKET* packet = reinterpret_cast<SC_CHAT_PACKET*>(buffer.data());
			m_vChat.push_back(packet->mess);
			if (m_vChat.size() > 4) {
				m_vChat.erase(m_vChat.begin());
			}
			buffer.erase(buffer.begin(), buffer.begin() + packet->size); 
			break;
		}
		case SC_STAT_CHANGE: { //player의 스탯상의 변화 패킷
			SC_STAT_CHANGE_PACKET* packet = reinterpret_cast<SC_STAT_CHANGE_PACKET*>(buffer.data());
			m_vSceneObject[m_playerId]->m_hp = packet->hp;
			m_vSceneObject[m_playerId]->m_maxhp = packet->max_hp;
			m_vSceneObject[m_playerId]->m_level = packet->level;
			m_vSceneObject[m_playerId]->m_exp = packet->exp;
			buffer.erase(buffer.begin(), buffer.begin() + sizeof(SC_STAT_CHANGE_PACKET));
			break;
		}
		default: {
			assert(0); // 패킷 유형이 올바르지 않음.
		}
			break;
		}
	}
}


POINT** GameFrameWork::GetPanPosition()
{
	return PanPosition;
}

CTile** GameFrameWork::GetMap()
{
	return m_Map;
}

vector<shared_ptr<Object>>& GameFrameWork::GetSceneObjects()
{
	return m_vSceneObject;
}

void GameFrameWork::FindNearestObject()
{
	// 가까운 NPC 찾기.
	static int m_old_nearest_obj_id = -1;
	int nearest_obj_id = -1;
	int near_range = 99999999;

	for (auto& o_id : m_viewObjects) {
		if (o_id < MAX_USER) continue; // NPC 만 찾도록한다.
		if (o_id == m_playerId) continue;
		auto other_pos = m_vSceneObject[o_id]->GetPosition();
		auto my_pos = m_vSceneObject[m_playerId]->GetPosition();
		Vector2 AtoB;
		AtoB.x = other_pos.x - my_pos.x;
		AtoB.y = other_pos.y - my_pos.y;
		int range = (AtoB.x * AtoB.x) + (AtoB.y * AtoB.y);
		if (near_range > range) {
			near_range = range;
			nearest_obj_id = o_id;
		}
	}

	if (nearest_obj_id != -1) {
		if (m_old_nearest_obj_id != nearest_obj_id && m_old_nearest_obj_id != -1) {
			m_vSceneObject[m_old_nearest_obj_id]->SetOriginBodyColor();
		}
		m_old_nearest_obj_id = nearest_obj_id;
		m_vSceneObject[nearest_obj_id]->SetBodyColor(COLORREF(RGB(0, 255, 0)));
		m_nearest_obj_id = nearest_obj_id;
	}
}