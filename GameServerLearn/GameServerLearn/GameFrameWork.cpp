#include "stdafx.h"
#include "GameFrameWork.h"
#include "Server.h"

POINT** GameFrameWork::PanPosition;

GameFrameWork::GameFrameWork()
{
	if (!PanPosition) {
		PanPosition = new POINT * [ROW_X];
		for (int i = 0;i < ROW_X;++i) {
			PanPosition[i] = new POINT[ROW_X];
		}
	}

	m_vSceneObject.push_back(make_unique<Player>());

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

void GameFrameWork::AddPlayerObject()
{
	m_vSceneObject.push_back(make_unique<Player>());
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
	vector<BYTE> buffer = serverFramework.GetRecvBuffer();

	BYTE playercount;
	memcpy(&playercount, buffer.data(), sizeof(BYTE));
	buffer.erase(buffer.begin(), buffer.begin() + sizeof(BYTE));

	PlayerInfo* playerinfo = new PlayerInfo[playercount];

	int stride = sizeof(int);
	for (int i = 0; i < playercount;++i) {
		memcpy(&playerinfo[i], buffer.data(), sizeof(PlayerInfo));
		buffer.erase(buffer.begin(), buffer.begin() + sizeof(PlayerInfo));
	}

	if (playercount != m_vSceneObject.size()) {
		while (true)
		{
			if (playercount == m_vSceneObject.size()) {
				break;
			}
			if (playercount > m_vSceneObject.size()) {
				AddPlayerObject();
			}
			else {
				DelPlayerObject();
			}
		}
	}
	for (int i = 0;i < playercount;++i){
		m_vSceneObject[i]->SetPosition(playerinfo[i].pos);
	}


	delete[] playerinfo;
}


POINT** GameFrameWork::GetPanPosition()
{
	return PanPosition;
}

vector<unique_ptr<Object>>& GameFrameWork::GetSceneObjects()
{
	return m_vSceneObject;
}
