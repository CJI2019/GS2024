// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <memory>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <chrono>
#include <cassert>

#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib,"WS2_32.LIB")

using namespace std::chrono;
using namespace std;

#define ROW_X 8 // 정사각형이므로 하나만 사용해도 무방
#define COL_Y 8