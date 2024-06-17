#pragma once
#include "stdafx.h"


WCHAR* CharToWCHAR(const char* cStr)
{
    // ��ȯ�� ���̵� ���ڿ��� ������ ����
    int requiredSize = MultiByteToWideChar(CP_ACP, 0, cStr, -1, NULL, 0);
    WCHAR* wstr = (WCHAR*)malloc(requiredSize * sizeof(WCHAR));

    // ���� ����Ʈ ���ڿ��� ���̵� ���ڿ��� ��ȯ
    MultiByteToWideChar(CP_ACP, 0, cStr, -1, wstr, requiredSize);

    return wstr;
}