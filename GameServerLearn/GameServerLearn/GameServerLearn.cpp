#pragma once
#include "stdafx.h"
#include "GameServerLearn.h"
#include "stdafx.h"
#include "Player.h"
#include "GameFrameWork.h"
#include "Server.h"

#define MAX_LOADSTRING 100

GameFrameWork gGameFrameWork;

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

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

UINT16 gRectHeightSize, gRectWidthSize;

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
            HBRUSH hBrush;
            HPEN hPen, hOldPen;
            int rectSize = gRectHeightSize / ROW_X; // 체스판 한 칸의 크기
            int boardSize = rectSize * ROW_X;       // 체스판 전체 크기

            int centerCustomXpos = (gRectWidthSize - gRectHeightSize) / 2;
            POINT** panPosition = gGameFrameWork.GetPanPosition();
            // 체스판 그리기
            for (int y = 0; y < ROW_X; y++) {
                for (int x = 0; x < ROW_X; x++) {
                    rect.left = (x * rectSize) + centerCustomXpos;
                    rect.top = y * rectSize;
                    rect.right = ((x + 1) * rectSize) + centerCustomXpos;
                    rect.bottom = (y + 1) * rectSize;

                    // 검은색과 흰색 칸 번갈아가며 색칠
                    hBrush = (x + y) % 2 == 0 ? (HBRUSH)GetStockObject(WHITE_BRUSH) : (HBRUSH)GetStockObject(BLACK_BRUSH);
                    FillRect(MemDC, &rect, hBrush);

                    panPosition[x][y] = { (rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2 };
                }
            }
            // 외곽선을 위한 펜 생성
            hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // 검은색 펜
            hOldPen = (HPEN)SelectObject(MemDC, hPen);
            hBrush = CreateSolidBrush(RGB(0, 0, 0));
            HBRUSH hOldBrush = (HBRUSH)SelectObject(MemDC, hBrush);

            // 체스판 외곽선 그리기
            rect.left = 0  + centerCustomXpos;
            rect.top = 0;
            rect.right = boardSize + centerCustomXpos;
            rect.bottom = boardSize;
            FrameRect(MemDC, &rect, hBrush);

            // 오브젝트 그리기
            for (auto& object : gGameFrameWork.GetSceneObjects()) {
                if (object->GetId() == -1) continue;
                object->Draw(MemDC, rectSize);
            }

            // 리소스 정리
            SelectObject(MemDC, hOldBrush);
            DeleteObject(hBrush);
            SelectObject(MemDC, hOldPen);
            DeleteObject(hPen);

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
