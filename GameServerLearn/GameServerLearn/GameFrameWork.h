#pragma once
#include "Player.h"

#define DIR_LEFT					0x01
#define DIR_RIGHT					0x02
#define DIR_UP						0x04
#define DIR_DOWN					0x08

class GameFrameWork
{
private:
	static POINT** PanPosition;

	vector<unique_ptr<Object>> m_vSceneObject; 

	int m_playerId = -1;//  m_vSceneObject[m_playerId] 로 플레이어를 접근
public:
	GameFrameWork();
	~GameFrameWork();

	void Update(float elapsedTime);
	void WriteData();
	void KeyInput(float elapsedTime);

	void AddPlayerObject(void* buffer);
	void DelPlayerObject();

	static POINT** GetPanPosition();
	vector<unique_ptr<Object>>& GetSceneObjects();
};

