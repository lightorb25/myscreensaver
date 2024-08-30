#include <windows.h>
#include <vector>
#include "resource.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

#define TIMER_ID 1
#define WM_MY_CUSTOM_MESSAGE (WM_USER + 1)

struct WindowData {
    int number;
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
std::vector<HWND> hWnds;

const wchar_t szWindowClass[] = L"ScreenSaverWindowClass";

struct MonitorInfo {
    HMONITOR hMonitor;
    RECT monitorRect;
};

std::vector<MonitorInfo> monitors;
HWND hWnd;
bool ignoreInput = false;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    MonitorInfo mi;
    mi.hMonitor = hMonitor;
    mi.monitorRect = *lprcMonitor;
    monitors.push_back(mi);
    return TRUE;
}

void CreateScreenSaverWindow(HINSTANCE hInstance, RECT rect) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    RegisterClassEx(&wcex);

    hWnd = CreateWindowEx(
        WS_EX_TOPMOST,
        szWindowClass,
        L"ScreenSaver",
        WS_POPUP,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hWnd) {
        WindowData* pData = new WindowData();
        pData->number = 0;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pData));

        hWnds.push_back(hWnd);
        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
    }
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    {
        bool showSettingsDialog = false;
        bool isInPreviewWindow = false;

        std::wstring s = std::wstring(lpCmdLine).substr(0, 2);
        if (s == L"\\c" || s == L"\\C" || s == L"/c" || s == L"/C") {
            showSettingsDialog = true;
        }
        else if (s == L"\\s" || s == L"\\S" || s == L"/s" || s == L"/S") {
            showSettingsDialog = false;
        }
        else if (s == L"\\p" || s == L"\\P" || s == L"/p" || s == L"/P") {
            showSettingsDialog = false;
            isInPreviewWindow = true;
        }

        if (isInPreviewWindow) {
            return 0;
        }

        if (showSettingsDialog) {
            MessageBox(NULL, L"Made By :D", L"Info", MB_OK);
            return 0;
        }
    }

    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, 0);
    for (const auto& monitor : monitors) {
        CreateScreenSaverWindow(hInstance, monitor.monitorRect);
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

void DrawBitmap(HDC hdc, HBITMAP hBitmap, int xStart, int yStart, int width, int height) {
    BITMAP bm;
    HDC hdcMem = CreateCompatibleDC(hdc);
    HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);

    GetObject(hBitmap, sizeof(bm), &bm);
    StretchBlt(hdc, xStart, yStart, width, height, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    SelectObject(hdcMem, oldBitmap);
    DeleteDC(hdcMem);
}

int screen_number =0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HBITMAP hBitmap = nullptr();
    static HBITMAP hBitmap0 = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_BITMAP1));
    static HBITMAP hBitmap1 = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_BITMAP2));
    static HBITMAP hBitmap2 = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_BITMAP3));

    switch (message) {
    case WM_MY_CUSTOM_MESSAGE: {
        WindowData* pData = reinterpret_cast<WindowData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pData) {
            pData->number = pData->number + 1;
            if (pData->number == 3) pData->number = 0;
        }
            for (HWND wnd : hWnds) {
                InvalidateRect(wnd, NULL, TRUE);
            }
        }
        break;
    case WM_CREATE:
        SetTimer(hWnd, TIMER_ID, 40000, NULL);
        break;
    case WM_TIMER:
        if (wParam == TIMER_ID) {
            PostMessage(hWnd, WM_MY_CUSTOM_MESSAGE, wParam, lParam);
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, (HBRUSH)(BLACK_BRUSH));
        WindowData* pData = reinterpret_cast<WindowData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pData->number == 2) { 
            hBitmap = hBitmap2;
        }
        else if (pData->number == 1) {
            hBitmap = hBitmap1;
        }
        else if (pData->number == 0) {
            hBitmap = hBitmap0;
        }
 
        if (hBitmap) {
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            DrawBitmap(hdc, hBitmap, 0, 0, width, height);
        }
        break;
    }
    case WM_MOUSEMOVE: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        static int startX;
        static int startY;
        static int timesCalled = 0; 
        if (timesCalled < 1) {
            startX = x;
            startY = y;
        }
        else if (startX != x || startY != y) { 
            ::PostQuitMessage(0);
        }
        timesCalled++;
        break;
    }
    case WM_KEYDOWN:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        if (!ignoreInput) {
            PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        {
            WindowData* pData = reinterpret_cast<WindowData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
            if (pData) {
                delete pData;
            }
        }
        if (hBitmap0) {
            DeleteObject(hBitmap0);
        }
        if (hBitmap1) {
            DeleteObject(hBitmap1);
        }
        if (hBitmap2) {
            DeleteObject(hBitmap1);
        }
        KillTimer(hWnd, TIMER_ID);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}