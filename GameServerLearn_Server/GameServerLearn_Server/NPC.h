#pragma once
#include "Object.h"

class NPC : public Object
{
public:
	NPC();
	~NPC();

	void Send_add_object(const int& c_id,const int& visual);

	void Move(char dir);
protected:
	// NPC는 활성화 한번할때 타이머 큐에 이벤트를 넣을것이므로
	// 중복된 이벤트를 넣으면 안됨.
	std::atomic_bool m_isActive;
};

