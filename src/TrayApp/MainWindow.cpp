#include "pch.h"
#include "MainWindow.h"
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <thread>

using namespace winrt::TrayApp::implementation;

const UINT WM_TRAY_NOTIFICATION = WM_APP + 1;
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
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr
    );
    
    // Регистрируем сообщение для пересоздания панели задач
    static UINT s_uTaskbarRestart = RegisterWindowMessageW(L"TaskbarCreated");
    
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
    
    AppendMenuW(hMenu, MF_STRING, 1, L"Открыть");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, 2, L"Выход");
    
    SetForegroundWindow(m_messageWindow);
    
    UINT cmd = TrackPopupMenu(
        hMenu,
        TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD,
        pt.x,
        pt.y,
        0,
        m_messageWindow,
        nullptr
    );
    
    DestroyMenu(hMenu);
    
    switch (cmd)
    {
    case 1: // Открыть
        ShowMainWindow();
        break;
    case 2: // Выход
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
            WNDCLASSW mainWc = {};
            mainWc.lpfnWndProc = DefWindowProcW;
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
                nullptr,
                GetModuleHandleW(nullptr),
                nullptr
            );
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
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
    }
    PostQuitMessage(0);
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
        return g_pThis->HandleMessage(msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
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
        switch (lParam)
        {
        case WM_LBUTTONUP:
            ShowMainWindow();
            break;
        case WM_RBUTTONUP:
            ShowTrayMenu();
            break;
        }
        return 0;
    }
    
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        
    default:
        return DefWindowProcW(msg, wParam, lParam);
    }
}

HRESULT STDMETHODCALLTYPE MainWindow::QueryInterface(REFIID riid, void** ppvObject)
{
    if (!ppvObject) return E_INVALIDARG;
    
    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }
    
    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE MainWindow::AddRef()
{
    return ++m_refCount;
}

ULONG STDMETHODCALLTYPE MainWindow::Release()
{
    ULONG refCount = --m_refCount;
    if (refCount == 0)
    {
        delete this;
    }
    return refCount;
}
