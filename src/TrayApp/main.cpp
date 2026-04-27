#include "pch.h"
#include "MainWindow.h"
#include <windows.h>

using namespace winrt::TrayApp::implementation;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
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
        // Создаем главное окно приложения
        auto mainWindow = new MainWindow();
        
        // Сообщение о том, что приложение запущено
        MessageBoxW(nullptr, L"Приложение запущено! Иконка находится в области уведомлений.", L"TrayApp", MB_ICONINFORMATION);
        
        // Обработка сообщений окна
        MSG msg = {};
        while (GetMessageW(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        
        // Очистка
        mainWindow->Release();
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
