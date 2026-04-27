#pragma once

#include "pch.h"
#include <windows.h>
#include <shellapi.h>

namespace trayapp
{
    class MainWindow
    {
    public:
        MainWindow();
        ~MainWindow();
        
        void InitTray();
        void ShowTrayMenu();
        void RestoreTrayIcon();
        void ShowMainWindow();
        void HideMainWindow();
        void HandleExit();
        
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        
        HWND GetHWND() const { return m_hWnd; }
        
    private:
        HWND m_hWnd = nullptr;
        HWND m_messageWindow = nullptr;
        NOTIFYICONDATA m_nid = {};
        bool m_isVisible = false;
    };
}
