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
	m_vSceneObject.reserve(MAX_USER);
	//m_vSceneObject.push_back(make_unique<Player>());
	for (int i = 0; i < MAX_USER;++i) {
		m_vSceneObject.emplace_back(make_unique<Player>());
	}
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

		m_vSceneObject[m_playerId]->Move(DIR, elapsedTime);
	}

}
int cnt = 0;
void GameFrameWork::AddPlayerObject(void* buffer)
{
	cnt++;
	SC_ADD_OBJECT_PACKET* packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(buffer);

	//int newId = -1;
	//for (auto& so : m_vSceneObject) {
	//	if (so->GetId() != -1) continue;
	//	newId = so->GetId();
	//	break;
	//}
	//if (newId == -1) { // 남은 자리가 없음 추가 불가능
	//	return;
	//}
	int newId = packet->id;
	m_vSceneObject[newId]->SetId(packet->id);
	m_vSceneObject[newId]->SetPosition(Vector2(packet->x, packet->y));
}

void GameFrameWork::DelPlayerObject()
{
	m_vSceneObject.erase(m_vSceneObject.end() - 1);
}

void GameFrameWork::Update(float elapsedTime)
{
	serverFramework.Logic();

	WriteData();

	KeyInput(elapsedTime);
}

void GameFrameWork::WriteData()
{
	auto buffer = serverFramework.GetRecvBuffer();

	while (buffer[0] != 0)
	{
		switch (buffer[2])
		{
		case SC_LOGIN_INFO: {
			SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(buffer.data());
			m_playerId = packet->id; // 메인 클라이언트 Id Set
			m_vSceneObject[m_playerId]->SetId(packet->id);
			m_vSceneObject[m_playerId]->SetPosition(Vector2(packet->x, packet->y));
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
		default:
			break;
		}

		if (buffer.size() == 0) {
			break;
		}
	}
	serverFramework.InitBuffer();
}


POINT** GameFrameWork::GetPanPosition()
{
	return PanPosition;
}

vector<unique_ptr<Object>>& GameFrameWork::GetSceneObjects()
{
	return m_vSceneObject;
}
