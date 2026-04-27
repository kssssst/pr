#include "pch.h"
#include "MainWindow.h"
#include <windows.h>

using namespace trayapp;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // Проверяем, запущено ли приложение уже
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, L"TrayApp_SingleInstance");
    
    if (hMutex == nullptr || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxW(nullptr, L"Приложение уже запущено!", L"TrayApp", MB_ICONINFORMATION);
        if (hMutex) CloseHandle(hMutex);
        return 1;
    }
    
    try
    {
        MainWindow mainWindow;
        
        // Обработка сообщений окна
        MSG msg = {};
        while (GetMessageW(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
        if (hMutex) CloseHandle(hMutex);
        return 1;
    }
    
    if (hMutex) CloseHandle(hMutex);
    return 0;
}
