#include "pch.h"
#include "ServiceClient.h"

#include <rpc.h>
#include <tlhelp32.h>
#include <windows.h>
#include <winsvc.h>

#include <cstdlib>
#include <cwchar>

namespace
{
constexpr wchar_t kServiceName[] = L"TrayService";
constexpr wchar_t kServiceExeName[] = L"TrayService.exe";
constexpr wchar_t kStopEventName[] = L"Global\\TrayServiceStopEvent";

bool QueryServiceState(SC_HANDLE service, DWORD& state)
{
    SERVICE_STATUS_PROCESS status = {};
    DWORD bytesNeeded = 0;
    if (!QueryServiceStatusEx(
            service,
            SC_STATUS_PROCESS_INFO,
            reinterpret_cast<LPBYTE>(&status),
            sizeof(status),
            &bytesNeeded))
    {
        return false;
    }

    state = status.dwCurrentState;
    return true;
}

bool WaitForServiceState(SC_HANDLE service, DWORD desiredState, DWORD timeoutMs)
{
    const DWORD startedAt = GetTickCount();

    for (;;)
    {
        DWORD state = 0;
        if (!QueryServiceState(service, state))
        {
            return false;
        }

        if (state == desiredState)
        {
            return true;
        }

        if (GetTickCount() - startedAt > timeoutMs)
        {
            return false;
        }

        Sleep(250);
    }
}

DWORD GetParentProcessId()
{
    const DWORD currentProcessId = GetCurrentProcessId();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);

    DWORD parentProcessId = 0;
    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            if (entry.th32ProcessID == currentProcessId)
            {
                parentProcessId = entry.th32ParentProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return parentProcessId;
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

namespace trayapp
{
bool EnsureServiceRunningOrExit()
{
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm)
    {
        return false;
    }

    SC_HANDLE service = OpenServiceW(scm, kServiceName, SERVICE_QUERY_STATUS);
    if (!service)
    {
        CloseServiceHandle(scm);
        return false;
    }

    DWORD state = 0;
    const bool stateRead = QueryServiceState(service, state);
    if (stateRead && state == SERVICE_RUNNING)
    {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return true;
    }

    if (stateRead && state != SERVICE_STOPPED)
    {
        const bool running = WaitForServiceState(service, SERVICE_RUNNING, 30000);
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return running;
    }

    CloseServiceHandle(service);

    service = OpenServiceW(scm, kServiceName, SERVICE_QUERY_STATUS | SERVICE_START);
    if (!service)
    {
        CloseServiceHandle(scm);
        return false;
    }

    if (!StartServiceW(service, 0, nullptr) && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
    {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return false;
    }

    const bool running = WaitForServiceState(service, SERVICE_RUNNING, 30000);
    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    return running;
}

bool IsParentProcessTrayService()
{
    const DWORD parentProcessId = GetParentProcessId();
    if (parentProcessId == 0)
    {
        return false;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);

    bool isService = false;
    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            if (entry.th32ProcessID == parentProcessId)
            {
                isService = _wcsicmp(entry.szExeFile, kServiceExeName) == 0;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return isService;
}

bool StopTrayServiceViaRpc()
{
    HANDLE stopEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, kStopEventName);
    if (!stopEvent)
    {
        return false;
    }

    const bool stopped = SetEvent(stopEvent) != FALSE;
    CloseHandle(stopEvent);
    return stopped;
}
}
