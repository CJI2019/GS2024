#include "stdafx.h"
#include "Player.h"
#include "GameFrameWork.h"
#include "Server.h"

Player::Player()
{
    m_fMoveLimit = 0.0f;
}

Player::~Player()
{
}

void Player::Draw(HDC& hdc,int rectSize) const
{
    // Ȳ�ݻ� �� �׸���
    // Ȳ�ݻ� �귯�� ����
    COLORREF goldColor = RGB(255, 215, 0); // Ȳ�ݻ� ����
    HBRUSH hBrush = CreateSolidBrush(goldColor);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // ���� �ܰ����� ���� �� ����
    HPEN hPen = CreatePen(PS_SOLID, 1, goldColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    // ü������ �߾ӿ� ���� ��ġ
    POINT curpos = GetPosition();
    POINT** PanPos = GameFrameWork::GetPanPosition();

    POINT pos = PanPos[curpos.x][curpos.y];

    UINT8 interval = rectSize / 2;
    Ellipse(hdc, pos.x - interval, pos.y - interval, pos.x + interval, pos.y + interval);

    // ���ҽ� ����
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void Player::Move(DWORD Dir,float elapsedTime)
{
    if (m_fMoveLimit > 0.0f) {
        m_fMoveLimit -= elapsedTime;
        return;
    }
    POINT curPos = GetPosition();

    // �÷��̾�� ��ĭ���� ������.
    if (Dir & DIR_RIGHT) {
        BYTE cmd = (BYTE)GameCommandList::MOVE;
        serverFramework.SendReserve(&cmd, sizeof(BYTE));
        cmd = (BYTE)PlayerMoveDir::RIGHT;
        serverFramework.SendReserve(&cmd, sizeof(BYTE));
    }
    else if (Dir & DIR_LEFT) {
        BYTE cmd = (BYTE)GameCommandList::MOVE;
        serverFramework.SendReserve(&cmd, sizeof(BYTE));
        cmd = (BYTE)PlayerMoveDir::LEFT;
        serverFramework.SendReserve(&cmd, sizeof(BYTE));
    }
    else if (Dir & DIR_UP) {
        BYTE cmd = (BYTE)GameCommandList::MOVE;
        serverFramework.SendReserve(&cmd, sizeof(BYTE));
        cmd = (BYTE)PlayerMoveDir::UP;
        serverFramework.SendReserve(&cmd, sizeof(BYTE));
    }
    else if (Dir & DIR_DOWN) {
        BYTE cmd = (BYTE)GameCommandList::MOVE;
        serverFramework.SendReserve(&cmd, sizeof(BYTE));
        cmd = (BYTE)PlayerMoveDir::DOWN;
        serverFramework.SendReserve(&cmd, sizeof(BYTE));
    }

    m_fMoveLimit = 0.05f;

}
