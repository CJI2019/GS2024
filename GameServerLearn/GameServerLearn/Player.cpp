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

    PlayerMoveDir p_Dir = PlayerMoveDir::NONE;
    // �÷��̾�� ��ĭ���� ������.
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
        packet.size = sizeof(CS_MOVE_PACKET);
        packet.Dir = p_Dir;
        serverFramework.SendReserve(&packet, packet.size);
    }

    m_fMoveLimit = 0.05f;

}
