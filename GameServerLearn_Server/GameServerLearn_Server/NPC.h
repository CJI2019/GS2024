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
	// NPC�� Ȱ��ȭ �ѹ��Ҷ� Ÿ�̸� ť�� �̺�Ʈ�� �������̹Ƿ�
	// �ߺ��� �̺�Ʈ�� ������ �ȵ�.
	std::atomic_bool m_isActive;
};

