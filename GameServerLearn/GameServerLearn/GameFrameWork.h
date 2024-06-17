#pragma once
#include "Player.h"
#include "Map.h"

#define DIR_LEFT					0x01
#define DIR_RIGHT					0x02
#define DIR_UP						0x04
#define DIR_DOWN					0x08

#define ATTACKCOOLTIME 1.0f

class GameFrameWork
{
private:
	static POINT** PanPosition;
	CTile** m_Map;
	vector<shared_ptr<Object>> m_vSceneObject; 

	int m_playerId = -1;//  m_vSceneObject[m_playerId] 로 플레이어를 접근
	int m_objectCount;
	int m_nearest_obj_id = -1;

	vector<string> m_vChat;
public:
	GameFrameWork();
	~GameFrameWork();

	void InitMap();
	void Update(float elapsedTime);
	void WriteData();
	void KeyInput(float elapsedTime);

	void AddPlayerObject(void* buffer);
	void DelPlayerObject(void* buffer);

	static POINT** GetPanPosition();
	CTile** GetMap();
	vector<shared_ptr<Object>>& GetSceneObjects();
	std::unordered_set<int> m_viewObjects;

	int GetMainPlayerId() { return m_playerId; }
	int GetObjectCount() { return m_objectCount; }

	void FindNearestObject();

	vector<string>& GetChatingBuffer() { return m_vChat; }
};

