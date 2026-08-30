#include "MemoryPatcher.h"
#include <Windows.h>

struct ModuleGuard 
{
    HMODULE hModule;
    ModuleGuard(HMODULE h) : hModule(h) {}
    ~ModuleGuard() { if (hModule) FreeLibrary(hModule); } 
};

PVOID MemoryPatcher::ScanPatch(LPCSTR funcName)
{
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return nullptr;

    HMODULE ntdllMapped = LoadLibraryExA("ntdll.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (!ntdllMapped) return nullptr; 

    // recreated for free library to be called whenever out of scope
    ModuleGuard guard(ntdllMapped); 

    try
    {
        auto hookedFunc = (PVOID)GetProcAddress(ntdll, funcName);
        if (!hookedFunc) return nullptr; 

        DWORD64 moduleBase = (DWORD64)ntdll;
        auto funcData = RtlLookupFunctionEntry((DWORD64)hookedFunc, &moduleBase, nullptr);
        if (!funcData) return nullptr;   

        auto funcSize = funcData->EndAddress - funcData->BeginAddress;
        auto originalFunc = (PVOID)GetProcAddress(ntdllMapped, funcName);
        if (!originalFunc) return nullptr; 

        auto result = RtlCompareMemory(hookedFunc, originalFunc, funcSize);

        if (result != funcSize)
        {
            RunPatch(hookedFunc, originalFunc, funcSize);
        }

        return hookedFunc; 
    }
    catch (...)
    {
        return nullptr;
    }
}

size_t MemoryPatcher::RunPatch(PVOID hookedFunc, PVOID originalFunc, DWORD funcSize)
{
    DWORD oldprotect = 0;

    VirtualProtect(hookedFunc, funcSize, PAGE_EXECUTE_READWRITE, &oldprotect);
    RtlCopyMemory(hookedFunc, originalFunc, funcSize);

    size_t result = RtlCompareMemory(hookedFunc, originalFunc, funcSize);

    DWORD dummy = 0;
    VirtualProtect(hookedFunc, funcSize, oldprotect, &dummy);

    return result;
}

void MemoryPatcher::PatchAll()
{
    const char* functionsToPatch[] = {
        "NtReadVirtualMemory",
        "NtWriteVirtualMemory",
        "NtProtectVirtualMemory",
        "NtQuerySystemInformation",
        "NtOpenProcess",
        "NtAllocateVirtualMemory",
        "NtFreeVirtualMemory"
    };

    for (const char* funcName : functionsToPatch) 
    {
        ScanPatch(funcName);
    }
}