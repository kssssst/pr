#include <windows.h>
#include <rpc.h>
#include <userenv.h>
#include <wtsapi32.h>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#include "TrayServiceRpc.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "userenv.lib")
#pragma comment(lib, "wtsapi32.lib")

namespace
{
constexpr wchar_t kServiceName[] = L"TrayService";
constexpr wchar_t kRpcEndpoint[] = L"TrayServiceRpcAlpc";

SERVICE_STATUS_HANDLE g_statusHandle = nullptr;
SERVICE_STATUS g_status = {};
HANDLE g_stopEvent = nullptr;
CRITICAL_SECTION g_processLock;

struct AppProcess
{
    DWORD sessionId = 0;
    DWORD processId = 0;
    HANDLE process = nullptr;
};

std::vector<AppProcess> g_processes;

void SetStatus(DWORD state, DWORD win32ExitCode = NO_ERROR, DWORD waitHint = 0)
{
    g_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_status.dwCurrentState = state;
    g_status.dwControlsAccepted =
        (state == SERVICE_RUNNING) ? SERVICE_ACCEPT_SESSIONCHANGE : 0;
    g_status.dwWin32ExitCode = win32ExitCode;
    g_status.dwWaitHint = waitHint;
    SetServiceStatus(g_statusHandle, &g_status);
}

std::wstring GetModuleDirectory()
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);

    std::wstring result(path);
    const size_t slash = result.find_last_of(L"\\/");
    if (slash != std::wstring::npos)
    {
        result.resize(slash);
    }
    return result;
}

bool IsProcessRunning(HANDLE process)
{
    if (!process)
    {
        return false;
    }

    return WaitForSingleObject(process, 0) == WAIT_TIMEOUT;
}

bool AlreadyStartedForSession(DWORD sessionId)
{
    EnterCriticalSection(&g_processLock);

    g_processes.erase(
        std::remove_if(
            g_processes.begin(),
            g_processes.end(),
            [](const AppProcess& item)
            {
                if (IsProcessRunning(item.process))
                {
                    return false;
                }
                CloseHandle(item.process);
                return true;
            }),
        g_processes.end());

    const bool found = std::any_of(
        g_processes.begin(),
        g_processes.end(),
        [sessionId](const AppProcess& item)
        {
            return item.sessionId == sessionId;
        });

    LeaveCriticalSection(&g_processLock);
    return found;
}

bool LaunchGuiForSession(DWORD sessionId)
{
    if (sessionId == 0 || AlreadyStartedForSession(sessionId))
    {
        return false;
    }

    HANDLE userToken = nullptr;
    if (!WTSQueryUserToken(sessionId, &userToken))
    {
        return false;
    }

    HANDLE primaryToken = nullptr;
    const BOOL duplicated = DuplicateTokenEx(
        userToken,
        TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID,
        nullptr,
        SecurityIdentification,
        TokenPrimary,
        &primaryToken);
    CloseHandle(userToken);

    if (!duplicated)
    {
        return false;
    }

    void* environment = nullptr;
    CreateEnvironmentBlock(&environment, primaryToken, FALSE);

    const std::wstring appPath = GetModuleDirectory() + L"\\TrayApp.exe";
    std::wstring commandLine = L"\"" + appPath + L"\"";

    STARTUPINFOW startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpDesktop = const_cast<LPWSTR>(L"winsta0\\default");

    PROCESS_INFORMATION processInfo = {};
    const BOOL created = CreateProcessAsUserW(
        primaryToken,
        appPath.c_str(),
        commandLine.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_UNICODE_ENVIRONMENT,
        environment,
        GetModuleDirectory().c_str(),
        &startupInfo,
        &processInfo);

    if (environment)
    {
        DestroyEnvironmentBlock(environment);
    }
    CloseHandle(primaryToken);

    if (!created)
    {
        return false;
    }

    CloseHandle(processInfo.hThread);

    EnterCriticalSection(&g_processLock);
    g_processes.push_back({ sessionId, processInfo.dwProcessId, processInfo.hProcess });
    LeaveCriticalSection(&g_processLock);

    return true;
}

void LaunchGuiForExistingSessions()
{
    WTS_SESSION_INFOW* sessions = nullptr;
    DWORD count = 0;

    if (!WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessions, &count))
    {
        return;
    }

    for (DWORD i = 0; i < count; ++i)
    {
        if (sessions[i].SessionId != 0)
        {
            LaunchGuiForSession(sessions[i].SessionId);
        }
    }

    WTSFreeMemory(sessions);
}

void StopAllGuiProcesses()
{
    EnterCriticalSection(&g_processLock);
    const auto processes = g_processes;
    g_processes.clear();
    LeaveCriticalSection(&g_processLock);

    for (const AppProcess& item : processes)
    {
        if (IsProcessRunning(item.process))
        {
            TerminateProcess(item.process, 0);
            WaitForSingleObject(item.process, 5000);
        }
        CloseHandle(item.process);
    }
}

DWORD WINAPI RpcServerThread(LPVOID)
{
    RPC_STATUS status = RpcServerUseProtseqEpW(
        reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(L"ncalrpc")),
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
        reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(kRpcEndpoint)),
        nullptr);
    if (status != RPC_S_OK)
    {
        SetEvent(g_stopEvent);
        return status;
    }

    status = RpcServerRegisterIf2(
        TrayServiceRpc_v1_0_s_ifspec,
        nullptr,
        nullptr,
        RPC_IF_ALLOW_LOCAL_ONLY,
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        static_cast<unsigned int>(-1),
        nullptr);
    if (status != RPC_S_OK)
    {
        SetEvent(g_stopEvent);
        return status;
    }

    status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);
    SetEvent(g_stopEvent);
    return status;
}

DWORD WINAPI ServiceControlHandler(DWORD control, DWORD eventType, LPVOID eventData, LPVOID)
{
    if (control != SERVICE_CONTROL_SESSIONCHANGE)
    {
        return NO_ERROR;
    }

    auto* notification = static_cast<WTSSESSION_NOTIFICATION*>(eventData);
    if (!notification || notification->dwSessionId == 0)
    {
        return NO_ERROR;
    }

    switch (eventType)
    {
    case WTS_SESSION_LOGON:
    case WTS_SESSION_UNLOCK:
    case WTS_CONSOLE_CONNECT:
    case WTS_REMOTE_CONNECT:
        LaunchGuiForSession(notification->dwSessionId);
        break;
    default:
        break;
    }

    return NO_ERROR;
}

void WINAPI ServiceMain(DWORD, LPWSTR*)
{
    g_statusHandle = RegisterServiceCtrlHandlerExW(kServiceName, ServiceControlHandler, nullptr);
    if (!g_statusHandle)
    {
        return;
    }

    SetStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    InitializeCriticalSection(&g_processLock);
    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_stopEvent)
    {
        SetStatus(SERVICE_STOPPED, GetLastError());
        DeleteCriticalSection(&g_processLock);
        return;
    }

    HANDLE rpcThread = CreateThread(nullptr, 0, RpcServerThread, nullptr, 0, nullptr);
    if (!rpcThread)
    {
        SetStatus(SERVICE_STOPPED, GetLastError());
        CloseHandle(g_stopEvent);
        DeleteCriticalSection(&g_processLock);
        return;
    }

    LaunchGuiForExistingSessions();
    SetStatus(SERVICE_RUNNING);

    WaitForSingleObject(g_stopEvent, INFINITE);
    SetStatus(SERVICE_STOP_PENDING, NO_ERROR, 5000);

    RpcMgmtStopServerListening(nullptr);
    WaitForSingleObject(rpcThread, 5000);
    RpcServerUnregisterIf(TrayServiceRpc_v1_0_s_ifspec, nullptr, FALSE);

    StopAllGuiProcesses();

    CloseHandle(rpcThread);
    CloseHandle(g_stopEvent);
    DeleteCriticalSection(&g_processLock);

    SetStatus(SERVICE_STOPPED);
}
}

extern "C" void __RPC_FAR* __RPC_USER midl_user_allocate(size_t size)
{
    return std::malloc(size);
}

extern "C" void __RPC_USER midl_user_free(void __RPC_FAR* pointer)
{
    std::free(pointer);
}

extern "C" void RpcStopTrayService(handle_t)
{
    if (g_stopEvent)
    {
        SetEvent(g_stopEvent);
    }
    RpcMgmtStopServerListening(nullptr);
}

int wmain()
{
    SERVICE_TABLE_ENTRYW serviceTable[] =
    {
        { const_cast<LPWSTR>(kServiceName), ServiceMain },
        { nullptr, nullptr }
    };

    if (!StartServiceCtrlDispatcherW(serviceTable))
    {
        return static_cast<int>(GetLastError());
    }

    return 0;
}
