#pragma once
#include "GameServerLearn.h"
#include "stdafx.h"
#include "Player.h"
#include "GameFrameWork.h"
#include "Server.h"

#define MAX_LOADSTRING 100

GameFrameWork gGameFrameWork;

UINT16 gRectHeightSize, gRectWidthSize;

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, HWND&, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAMESERVERLEARN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    HWND hWnd;

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, hWnd,nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMESERVERLEARN));
    
    MSG msg;
    gGameFrameWork.InitMap();

    auto lastTime = high_resolution_clock::now();

    // 메시지 루프
    while (TRUE) {
        auto currentTime = high_resolution_clock::now();

        duration<double> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;
        // 메시지 큐에 메시지가 있는지 확인
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            // 메시지 처리
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // WM_QUIT 메시지를 받으면 루프 종료
            if (msg.message == WM_QUIT) {
                break;
            }
        }
        else {
            // 메시지 큐에 메시지가 없을 때 수행할 작업
            gGameFrameWork.Update(static_cast<float>(elapsedTime.count()));

            InvalidateRect(hWnd, NULL, false);
            UpdateWindow(hWnd);
        }

    }


    return (int) msg.wParam;
}


//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMESERVERLEARN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GAMESERVERLEARN);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, HWND& hWnd, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   /*hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0,
       CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);*/
   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       0, 0,
       FRAME_WIDTH, FRAME_HEIGHT, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// 윈도우 프로시저 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        {
            int width = LOWORD(lParam);  // 창의 새 너비
            int height = HIWORD(lParam); // 창의 새 높이

            // 창 크기가 변경될때 창 크기를 메시지박스로 확인 
            TCHAR szSize[100];
            wsprintf(szSize, TEXT("Width: %d, Height: %d"), width, height);
            gRectWidthSize = width;
            gRectHeightSize = height;
            //MessageBox(hWnd, szSize, TEXT("창 크기 확인"), MB_OK);

            gGameFrameWork.InitMap();
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT: 
        {
            PAINTSTRUCT ps;
            HDC hdc, MemDC;
            HBITMAP BackBit, oldBackBit;
            RECT bufferRT;

            // 더블 버퍼링 시작 처리
            hdc = BeginPaint(hWnd, &ps);
            GetClientRect(hWnd, &bufferRT);
            MemDC = CreateCompatibleDC(hdc);
            BackBit = CreateCompatibleBitmap(hdc, bufferRT.right, bufferRT.bottom);
            oldBackBit = (HBITMAP)SelectObject(MemDC, BackBit);
            PatBlt(MemDC, 0, 0, bufferRT.right, bufferRT.bottom, WHITENESS);

            RECT rect;
            HBRUSH hBrush, hOldBrush;
            HPEN hPen, hOldPen;
            HFONT hOldFont = nullptr;

            int viewSize = 20;
            int rectSize = gRectHeightSize / viewSize; // 체스판 한 칸의 크기
            int boardSize = rectSize * viewSize;       // 체스판 전체 크기

            int centerCustomXpos = (gRectWidthSize - gRectHeightSize) / 2;

            POINT** panPosition = gGameFrameWork.GetPanPosition();
            auto& SceneObejcts = gGameFrameWork.GetSceneObjects();

            int B_offsetX = 0; // 보드의 색을 바꾸기 위한 옵션 0이면 흰색(0,0) 1이면 검은색(0,0)
            int B_offsetY = 0; 
            {
                int mainPlayer_Id = gGameFrameWork.GetMainPlayerId();
                if (mainPlayer_Id != -1) {
                    Vector2 mainPos = SceneObejcts[mainPlayer_Id]->GetPosition();

                    // 메인플레이어 좌표 표시

                    // 폰트를 선택하고 이전 폰트를 저장
                    hOldFont = (HFONT)SelectObject(MemDC, GetFont());
                    RECT rect = { 0, 100, 500, 300 }; // 텍스트가 그려질 사각형 영역
                    TCHAR text[30];
                    SetTextColor(MemDC, COLORREF(rgbBlue));
                    _stprintf_s(text, _T("좌표 : (%d, %d)"), mainPos.x, mainPos.y);
                    DrawText(MemDC, text, -1, &rect, DT_CENTER | DT_SINGLELINE);

                    if (mainPos.x > viewSize / 2 ) {
                        B_offsetX = (mainPos.x) % 2 == 0 ? 0 : 1;
                    }
                    if ( mainPos.y > viewSize / 2) {
                        B_offsetY = (mainPos.y) % 2 == 0 ? 0 : 1;
                    }
                }
            }
            // 체스판 그리기
            for (int y = 0; y < viewSize; y++) {
                for (int x = 0; x < viewSize; x++) {
                    rect.left = (x * rectSize) + centerCustomXpos;
                    rect.top = y * rectSize;
                    rect.right = ((x + 1) * rectSize) + centerCustomXpos;
                    rect.bottom = (y + 1) * rectSize;

                    // 검은색과 흰색 칸 번갈아가며 색칠
                    hBrush = (x + B_offsetX + y + B_offsetY) % 2 == 0 ? (HBRUSH)GetStockObject(GRAY_BRUSH) : (HBRUSH)GetStockObject(LTGRAY_BRUSH);
                    FillRect(MemDC, &rect, hBrush);

                    //panPosition[x][y] = { (rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2 };
                }
            }

            {
                hPen = CreatePen(PS_SOLID, 1, RGB(200, 0, 0)); // 검은색 펜
                hOldPen = (HPEN)SelectObject(MemDC, hPen);
                hBrush = CreateSolidBrush(RGB(200, 0, 0)); // 외곽선을 위한 브러시
                hOldBrush = (HBRUSH)SelectObject(MemDC, hBrush);

                // 체스판 외곽선 그리기
                int Frame_offset = 0;
                rect.left = 0 + centerCustomXpos - Frame_offset;
                rect.top = 0 - Frame_offset;
                rect.right = boardSize + centerCustomXpos + Frame_offset;
                rect.bottom = boardSize + Frame_offset;
                FrameRect(MemDC, &rect, hBrush);
            }

            int mainPlayer_Id = gGameFrameWork.GetMainPlayerId();
            if (mainPlayer_Id != -1) {
                Vector2 mainPos = SceneObejcts[mainPlayer_Id]->GetPosition();

                int offsetX = 0;
                if (mainPos.x > viewSize / 2) {
                    offsetX = rectSize * (mainPos.x - viewSize / 2);
                }

                int offsetY = 0;
                if (mainPos.y > viewSize / 2) {
                    offsetY = rectSize * (mainPos.y - viewSize / 2);
                }

                POINT offset = { offsetX,offsetY };

                // 오브젝트 그리기
                for (auto& object : SceneObejcts) {
                    if (object->GetId() == -1) continue;
                    object->Draw(MemDC, rectSize, offset);
                }
            }

            // 리소스 정리
            SelectObject(MemDC, hOldBrush);
            DeleteObject(hBrush);
            SelectObject(MemDC, hOldPen);
            DeleteObject(hPen);
            SelectObject(hdc, hOldFont);

            // 더블 버퍼링 끝 처리
            BitBlt(hdc, 0, 0, bufferRT.right, bufferRT.bottom, MemDC, 0, 0, SRCCOPY);
            SelectObject(MemDC, oldBackBit);
            DeleteObject(BackBit);
            DeleteDC(MemDC);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        serverFramework.~Server();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
