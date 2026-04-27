#pragma once

namespace trayapp
{
    bool EnsureServiceRunningOrExit();
    bool IsParentProcessTrayService();
    bool StopTrayServiceViaRpc();
}
