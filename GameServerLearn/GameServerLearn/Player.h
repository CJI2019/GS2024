#pragma once


class Object {
protected:
	Vector2 m_Pos = { 0,0 }; // 체스 판에 해당하는 좌표가 돼야함.

	int m_Id = -1;
	std::string m_name = "NoName";

	COLORREF m_bodyColor = RGB(0, 0, 0);
	COLORREF m_originbodyColor = RGB(0, 0, 0);
public:
    virtual void Draw(HDC& hdc, int rectSize, POINT& offset) const = 0;
	virtual void Move(DWORD Dir,float elapsedTime) = 0;

	void SetPosition(Vector2 pos) {
		m_Pos.x = pos.x;
		m_Pos.y = pos.y;
	}

	Vector2 GetPosition() const {
		return m_Pos;
	}

	int GetId() { return m_Id; }
	void SetId(int id) {
		m_Id = id;
	}
	void SetName(const char* name) {
		m_name = name;
	}
	void SetBodyColor(COLORREF color) {
		m_bodyColor = color;
	}
	void SetOriginBodyColor() {
		m_bodyColor = m_originbodyColor;
	}
	void SetVisual(int visual) {
		if (visual == visual_NPC1) {
			m_originbodyColor = RGB(200, 0, 0);
			m_bodyColor = RGB(200, 0, 0);
		}
		else if(visual== visual_NPC2) {
			m_originbodyColor = RGB(0, 0, 200);
			m_bodyColor = RGB(0, 0, 200);
		}
	}

	int m_visual = 0;

	int m_exp = 0;
	int m_level = 1;
	int m_hp = 100;
	int m_maxhp = 100;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Player : public Object
{
public:
	Player();
	~Player();

	void Draw(HDC& hdc, int rectSize, POINT& offset) const;
	void Move(DWORD Dir,float elapsedTime);
private:
	float m_fMoveLimit;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class NPC : public Object
{
public:
	NPC();
	~NPC();

	void Draw(HDC& hdc, int rectSize, POINT& offset) const;
	void Move(DWORD Dir, float elapsedTime) {};
private:

	// 비주얼에 따라서 외형 변경
	int m_visual;
	COLORREF m_Color = RGB(200, 0, 0);

};

