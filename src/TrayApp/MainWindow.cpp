#include "pch.h"
#include "MainWindow.h"
#include "ServiceClient.h"
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>

using namespace trayapp;

const UINT WM_TRAY_NOTIFICATION = WM_APP + 1;
const UINT ID_TRAY_OPEN = 1;
const UINT ID_TRAY_EXIT = 2;
const UINT ID_FILE_EXIT = 1001;
const wchar_t* CLASS_NAME = L"TrayAppMessageWindow";

// Глобальная переменная для доступа к объекту из WndProc
static MainWindow* g_pThis = nullptr;

MainWindow::MainWindow()
{
    g_pThis = this;
    m_isVisible = false;
    
    // Регистрируем класс окна
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = CLASS_NAME;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassW(&wc);
    
    // Создаем скрытое окно для получения сообщений о трее
    m_messageWindow = CreateWindowExW(
        0,
        CLASS_NAME,
        L"TrayApp Message Window",
        WS_OVERLAPPED,
        0, 0, 0, 0,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );
    
    if (!m_messageWindow)
    {
        throw std::runtime_error("Failed to create message window");
    }
    
    InitTray();
}

MainWindow::~MainWindow()
{
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
    if (m_messageWindow)
    {
        DestroyWindow(m_messageWindow);
    }
    g_pThis = nullptr;
}

void MainWindow::InitTray()
{
    ZeroMemory(&m_nid, sizeof(NOTIFYICONDATA));
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_messageWindow;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAY_NOTIFICATION;
    
    // Загружаем стандартную иконку
    m_nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wcscpy_s(m_nid.szTip, sizeof(m_nid.szTip) / sizeof(wchar_t), L"TrayApp");
    
    // Добавляем иконку в трей
    if (!Shell_NotifyIconW(NIM_ADD, &m_nid))
    {
        MessageBoxW(nullptr, L"Failed to add icon to tray", L"Error", MB_ICONERROR);
        return;
    }
    
    // Устанавливаем версию для поддержки balloon tips
    m_nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &m_nid);
}

void MainWindow::ShowTrayMenu()
{
    POINT pt;
    GetCursorPos(&pt);
    
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;
    
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_OPEN, L"Открыть");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Выход");
    
    SetForegroundWindow(m_messageWindow);
    
    UINT cmd = TrackPopupMenuEx(
        hMenu,
        TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
        pt.x,
        pt.y,
        m_messageWindow,
        nullptr
    );
    
    DestroyMenu(hMenu);
    PostMessageW(m_messageWindow, WM_NULL, 0, 0);
    
    switch (cmd)
    {
    case ID_TRAY_OPEN:
        ShowMainWindow();
        break;
    case ID_TRAY_EXIT:
        HandleExit();
        break;
    }
}

void MainWindow::ShowMainWindow()
{
    if (!m_isVisible)
    {
        // Создаем или показываем окно
        if (!m_hWnd)
        {
            HMENU fileMenu = CreatePopupMenu();
            AppendMenuW(fileMenu, MF_STRING, ID_FILE_EXIT, L"Выход");

            HMENU menuBar = CreateMenu();
            AppendMenuW(menuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu), L"Файл");

            WNDCLASSW mainWc = {};
            mainWc.lpfnWndProc = WndProc;
            mainWc.lpszClassName = L"TrayAppMainWindow";
            mainWc.hInstance = GetModuleHandleW(nullptr);
            mainWc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
            mainWc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            mainWc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            RegisterClassW(&mainWc);
            
            m_hWnd = CreateWindowExW(
                WS_EX_APPWINDOW,
                L"TrayAppMainWindow",
                L"TrayApp",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
                nullptr,
                menuBar,
                GetModuleHandleW(nullptr),
                nullptr
            );

            if (!m_hWnd)
            {
                DestroyMenu(menuBar);
                DestroyMenu(fileMenu);
            }
        }
        
        if (m_hWnd)
        {
            ShowWindow(m_hWnd, SW_SHOW);
            SetForegroundWindow(m_hWnd);
            m_isVisible = true;
        }
    }
    else if (m_hWnd)
    {
        ShowWindow(m_hWnd, SW_SHOW);
        SetForegroundWindow(m_hWnd);
    }
}

void MainWindow::HideMainWindow()
{
    if (m_isVisible && m_hWnd)
    {
        ShowWindow(m_hWnd, SW_HIDE);
        m_isVisible = false;
    }
}

void MainWindow::HandleExit()
{
    if (!StopTrayServiceViaRpc())
    {
        MessageBoxW(m_hWnd, L"Не удалось остановить службу TrayService.", L"TrayApp", MB_ICONERROR);
        return;
    }
}

void MainWindow::RestoreTrayIcon()
{
    Shell_NotifyIconW(NIM_ADD, &m_nid);
    m_nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &m_nid);
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_pThis)
    {
        return g_pThis->HandleMessage(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static UINT s_uTaskbarRestart = RegisterWindowMessageW(L"TaskbarCreated");
    
    if (msg == s_uTaskbarRestart)
    {
        // Панель задач пересоздана
        RestoreTrayIcon();
        return 0;
    }
    
    switch (msg)
    {
    case WM_TRAY_NOTIFICATION:
    {
        const UINT trayEvent = LOWORD(lParam);

        switch (trayEvent)
        {
        case WM_LBUTTONUP:
        case NIN_SELECT:
            ShowMainWindow();
            break;
        case WM_RBUTTONUP:
        case WM_CONTEXTMENU:
            ShowTrayMenu();
            break;
        }
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_FILE_EXIT)
        {
            HandleExit();
            return 0;
        }
        break;
    
    case WM_CLOSE:
        if (hwnd == m_hWnd)
        {
            HideMainWindow();
            return 0;
        }
        break;

    case WM_DESTROY:
        if (hwnd == m_messageWindow)
        {
            PostQuitMessage(0);
            return 0;
        }
        break;

    case WM_NCDESTROY:
        if (hwnd == m_hWnd)
        {
            m_hWnd = nullptr;
            m_isVisible = false;
            return 0;
        }
        break;

    default:
        break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
