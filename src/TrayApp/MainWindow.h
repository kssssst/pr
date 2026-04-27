#pragma once

#include "pch.h"
#include <windows.h>
#include <shellapi.h>

namespace winrt::TrayApp::implementation
{
    struct MainWindow : public IUnknown
    {
        MainWindow();
        ~MainWindow();
        
        void InitTray();
        void ShowTrayMenu();
        void RestoreTrayIcon();
        void ShowMainWindow();
        void HideMainWindow();
        void HandleExit();
        
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);
        
        HWND GetHWND() const { return m_hWnd; }
        
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
        ULONG STDMETHODCALLTYPE AddRef() override;
        ULONG STDMETHODCALLTYPE Release() override;
        
    private:
        HWND m_hWnd = nullptr;
        HWND m_messageWindow = nullptr;
        NOTIFYICONDATA m_nid = {};
        bool m_isVisible = false;
        long m_refCount = 1;
    };
}
