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
    // 황금색 말 그리기
    // 황금색 브러시 생성
    COLORREF goldColor = RGB(255, 215, 0); // 황금색 정의
    HBRUSH hBrush = CreateSolidBrush(goldColor);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // 말의 외곽선을 위한 펜 생성
    HPEN hPen = CreatePen(PS_SOLID, 1, goldColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    // 체스판의 중앙에 말을 배치
    POINT curpos = GetPosition();
    POINT** PanPos = GameFrameWork::GetPanPosition();

    POINT pos = PanPos[curpos.x][curpos.y];

    UINT8 interval = rectSize / 2;
    Ellipse(hdc, pos.x - interval, pos.y - interval, pos.x + interval, pos.y + interval);

    // 리소스 정리
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

    // 플레이어는 한칸씩만 움직임.
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
