#include <windows.h>
#include "Hooks.hpp"

HINSTANCE gDllInstance = NULL;

extern "C"
{
    BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
    {
        if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        {
            gDllInstance = hModule;
            InstallHooks();
        }
        return TRUE;
    }
}
