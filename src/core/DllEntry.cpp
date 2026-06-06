#include <Windows.h>
#include <thread>
#include "sdk/Logger/Logger.h"
#include "sdk/Memory/PatternScan.h"
#include "sdk/Memory/Memory.h"

// this gets ran multiple times
static void MainThread(HMODULE hModule)
{
    //Logger::Log("this is a log");
    Memory::GetModuleBase("ac_client.exe");


    // cleanup and eject DLL when done
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CloseHandle(CreateThread(nullptr, 0, 
            (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}