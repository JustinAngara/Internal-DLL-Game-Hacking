#include "MemoryPatcher.h"
#include <Windows.h>

PVOID MemoryPatcher::ScanPatch(LPCSTR funcName)
{
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    HMODULE ntdllMapped = LoadLibraryExA("ntdll.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);

    PVOID returnAddress = nullptr;
    if (!ntdll || !ntdllMapped) return nullptr; 

    try
    {
        DWORD64 moduleBase = (DWORD64)ntdll;
        auto hookedFunc = (PVOID)GetProcAddress(ntdll, funcName);
        auto funcData = RtlLookupFunctionEntry((DWORD64)hookedFunc, &moduleBase, nullptr);

        if (funcData) 
        {
            auto funcSize = funcData->EndAddress - funcData->BeginAddress;
            auto originalFunc = (PVOID)GetProcAddress(ntdllMapped, funcName);

            if (hookedFunc && originalFunc) 
            {
                auto result = RtlCompareMemory(hookedFunc, originalFunc, funcSize);

                if (result != funcSize)
                {
                    RunPatch(hookedFunc, originalFunc, funcSize);
                }

                returnAddress = hookedFunc;
            }
        }
    }
    catch (...)
    {
        return nullptr;
    }

    if (ntdllMapped) 
    {
        FreeLibrary(ntdllMapped);
    }
    return returnAddress;
}


size_t MemoryPatcher::RunPatch(PVOID hookedFunc, PVOID originalFunc, DWORD funcSize)
{
    DWORD oldprotect = 0;

    // un proc mem
    VirtualProtect(hookedFunc, funcSize, PAGE_EXECUTE_READWRITE, &oldprotect);

    // patch
    RtlCopyMemory(hookedFunc, originalFunc, funcSize);
    size_t result = RtlCompareMemory(hookedFunc, originalFunc, funcSize);

    // restore
    DWORD dummy = 0;
    VirtualProtect(hookedFunc, funcSize, oldprotect, &dummy);

    return result;
}