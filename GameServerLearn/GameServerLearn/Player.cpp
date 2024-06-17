#pragma once
#include "stdafx.h"
#include "Player.h"
#include "GameFrameWork.h"
#include "Server.h"

Player::Player()
{
    m_fMoveLimit = 0.0f;
    m_bodyColor = RGB(255, 215, 0);
    m_originbodyColor = RGB(255, 215, 0);
}

Player::~Player()
{
}

void Player::Draw(HDC& hdc,int rectSize,POINT& offset) const
{
    // 황금색 말 그리기
    // 황금색 브러시 생성
    
    HBRUSH hBrush = CreateSolidBrush(m_bodyColor);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // 말의 외곽선을 위한 펜 생성
    HPEN hPen = CreatePen(PS_SOLID, 1, m_bodyColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    // 체스판의 중앙에 말을 배치
    Vector2 curpos = GetPosition();
    POINT** PanPos = GameFrameWork::GetPanPosition();

    POINT pos = PanPos[curpos.x][curpos.y];
    pos.x -= offset.x;
    pos.y -= offset.y;
    UINT8 interval = rectSize / 2;
    Ellipse(hdc, pos.x - interval, pos.y - interval, pos.x + interval, pos.y + interval);
    TextOutA(hdc, pos.x - interval/2, pos.y - interval, m_name.c_str(),static_cast<int>(m_name.size())); //ANSI(8비트 Windows) 

    //DrawText(hdc)
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

    MoveDir p_Dir = MoveDir::NONE;
    // 플레이어는 한칸씩만 움직임.
    if (Dir & DIR_RIGHT) {
        p_Dir = MoveDir::RIGHT;
    }
    else if (Dir & DIR_LEFT) {
        p_Dir = MoveDir::LEFT;
    }
    else if (Dir & DIR_UP) {
        p_Dir = MoveDir::UP;
    }
    else if (Dir & DIR_DOWN) {
        p_Dir = MoveDir::DOWN;
    }

    if (p_Dir != MoveDir::NONE) {
        CS_MOVE_PACKET packet;
        packet.type = CS_MOVE;
        packet.size = sizeof(CS_MOVE_PACKET);
        packet.direction = static_cast<char>(p_Dir);
        serverFramework.SendReserve(&packet, packet.size);

        m_fMoveLimit = 1.0f;
    }


}

NPC::NPC()
{
    m_bodyColor = RGB(200, 0, 0);
    m_originbodyColor = RGB(200, 0, 0);
}

NPC::~NPC()
{
}

void NPC::Draw(HDC& hdc, int rectSize, POINT& offset) const
{
    HBRUSH hBrush = CreateSolidBrush(m_bodyColor);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // 말의 외곽선을 위한 펜 생성
    HPEN hPen = CreatePen(PS_SOLID, 1, m_bodyColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    // 체스판의 중앙에 말을 배치
    Vector2 curpos = GetPosition();
    POINT** PanPos = GameFrameWork::GetPanPosition();

    POINT pos = PanPos[curpos.x][curpos.y];
    pos.x -= offset.x;
    pos.y -= offset.y;
    UINT8 interval = rectSize / 2;
    Ellipse(hdc, pos.x - interval, pos.y - interval, pos.x + interval, pos.y + interval);
    TextOutA(hdc, pos.x - interval / 2, pos.y - interval, m_name.c_str(), static_cast<int>(m_name.size())); //ANSI(8비트 Windows) 

    // 리소스 정리
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}
