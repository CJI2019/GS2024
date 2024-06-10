#pragma once
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

void Player::Draw(HDC& hdc,int rectSize,POINT& offset) const
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
    Vector2 curpos = GetPosition();
    POINT** PanPos = GameFrameWork::GetPanPosition();

    POINT pos = PanPos[curpos.x][curpos.y];
    pos.x -= offset.x;
    pos.y -= offset.y;
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

    PlayerMoveDir p_Dir = PlayerMoveDir::NONE;
    // 플레이어는 한칸씩만 움직임.
    if (Dir & DIR_RIGHT) {
        p_Dir = PlayerMoveDir::RIGHT;
    }
    else if (Dir & DIR_LEFT) {
        p_Dir = PlayerMoveDir::LEFT;
    }
    else if (Dir & DIR_UP) {
        p_Dir = PlayerMoveDir::UP;
    }
    else if (Dir & DIR_DOWN) {
        p_Dir = PlayerMoveDir::DOWN;
    }

    if (p_Dir != PlayerMoveDir::NONE) {
        CS_MOVE_PACKET packet;
        packet.type = CS_MOVE;
        packet.size = sizeof(CS_MOVE_PACKET);
        packet.direction = static_cast<char>(p_Dir);
        serverFramework.SendReserve(&packet, packet.size);
    }

    m_fMoveLimit = 0.05f;

}
