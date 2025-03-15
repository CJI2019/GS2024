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
	int rectSize = gRectHeightSize / viewSize; // ü���� �� ĭ�� ũ��
	int boardSize = rectSize * viewSize;       // ü���� ��ü ũ��

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
	// ��ü�� �����͸� �ʱ�ȭ ��Ų��.
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
	//���� �������� ���� ó���� �ϹǷ�, ���� ����� ��븦 ��󳻴� �۾�����.
	//  -> �̹� �������� ���´µ� ������ �۾��� �ι��ϹǷ� �̴� ���� �������� ��Ŷ�� �޵��� �ٲ����.

	//auto Attack
	float m_attackCooltime = 0.0f;
	if (m_attackCooltime > ATTACKCOOLTIME) {
		// ��
	}
	m_attackCooltime += elapsedTime;
	if (m_nearest_obj_id != -1) { // ���� npc�� �����Ѵ�.
		// ���� ��Ŷ�� ������.
	}
}

void GameFrameWork::WriteData()
{
	auto buffer = serverFramework.GetRecvBuffer();

	unsigned short packet_size;

	while (buffer.size() != 0)
	{ // ��Ŷ ����� 2����Ʈ�̱� ������ �̿� ���� ó�� �����ؾ���.
		if (buffer.size() > 1) {
			memcpy(&packet_size, buffer.data(), sizeof(unsigned short));
			if (packet_size >= BUFSIZE) break;
			if (buffer.size() < packet_size) {
				serverFramework.remain_recv_byte += buffer.size();
				memcpy(serverFramework.packet_buf, buffer.data(), buffer.size());
				serverFramework.InitBuffer(buffer.size());
				break;
			} //[10] [3]�� ����. [3]�� �ش��ϴ� ���۸� packetbuf�� ������ ��ܿ;���.
			else if(packet_size == 0){
				serverFramework.InitBuffer(0);
				break;
			}
		}
		else { // buffer�� ����� 1����Ʈ ���ΰ���. 2����Ʈ �� 1����Ʈ�� ���� ���ۿ� �°��� ����.
			//���� 1����Ʈ�� ó��
			serverFramework.remain_recv_byte += 1;
			memcpy(serverFramework.packet_buf, buffer.data(), buffer.size());
			serverFramework.InitBuffer(1);
			break;
		}
		switch (buffer[2]) // ��Ŷ�� ������ ��Ÿ�� 1����Ʈ�� ǥ�� 256�� �̻��� ��Ŷ ������ �������� ���� ���̹Ƿ�..
		{
		case SC_LOGIN_INFO: {
			SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(buffer.data());
			m_playerId = packet->id; // ���� Ŭ���̾�Ʈ Id Set
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
		case SC_CHAT: { // ���� �޽��� �� ä�� ��Ŷ
			SC_CHAT_PACKET* packet = reinterpret_cast<SC_CHAT_PACKET*>(buffer.data());
			m_vChat.push_back(packet->mess);
			if (m_vChat.size() > 4) {
				m_vChat.erase(m_vChat.begin());
			}
			buffer.erase(buffer.begin(), buffer.begin() + packet->size); 
			break;
		}
		case SC_STAT_CHANGE: { //player�� ���Ȼ��� ��ȭ ��Ŷ
			SC_STAT_CHANGE_PACKET* packet = reinterpret_cast<SC_STAT_CHANGE_PACKET*>(buffer.data());
			m_vSceneObject[m_playerId]->m_hp = packet->hp;
			m_vSceneObject[m_playerId]->m_maxhp = packet->max_hp;
			m_vSceneObject[m_playerId]->m_level = packet->level;
			m_vSceneObject[m_playerId]->m_exp = packet->exp;
			buffer.erase(buffer.begin(), buffer.begin() + sizeof(SC_STAT_CHANGE_PACKET));
			break;
		}
		default: {
			assert(0); // ��Ŷ ������ �ùٸ��� ����.
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
	// ����� NPC ã��.
	static int m_old_nearest_obj_id = -1;
	int nearest_obj_id = -1;
	int near_range = 99999999;

	for (auto& o_id : m_viewObjects) {
		if (o_id < MAX_USER) continue; // NPC �� ã�����Ѵ�.
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