#pragma once


class Object {
private:
	POINT m_Pos = { 0,0 }; // 체스 판에 해당하는 좌표가 돼야함.

	int m_Id = -1;
public:
    virtual void Draw(HDC& hdc, int rectSize) const = 0;
	virtual void Move(DWORD Dir,float elapsedTime) = 0;

	void SetPosition(POINT pos) {
		m_Pos.x = pos.x;
		m_Pos.y = pos.y;
	}

	POINT GetPosition() const {
		return m_Pos;
	}

	int GetId() { return m_Id; }
	void SetId(int id) {
		m_Id = id;
	}

};

class Player : public Object
{
public:
	Player();
	~Player();

	void Draw(HDC& hdc, int rectSize) const;
	void Move(DWORD Dir,float elapsedTime);
private:
	float m_fMoveLimit;

};

