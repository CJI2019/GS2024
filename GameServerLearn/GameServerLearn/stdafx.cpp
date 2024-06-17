#pragma once
#include "stdafx.h"


WCHAR* CharToWCHAR(const char* cStr)
{
    // 변환된 와이드 문자열을 저장할 버퍼
    int requiredSize = MultiByteToWideChar(CP_ACP, 0, cStr, -1, NULL, 0);
    WCHAR* wstr = (WCHAR*)malloc(requiredSize * sizeof(WCHAR));

    // 다중 바이트 문자열을 와이드 문자열로 변환
    MultiByteToWideChar(CP_ACP, 0, cStr, -1, wstr, requiredSize);

    return wstr;
}