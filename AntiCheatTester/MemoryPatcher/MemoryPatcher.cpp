#include "MemoryPatcher.h"
#include <Windows.h>
#include "../ntdll/NtDllHandler.h"


typedef NTSTATUS(NTAPI* pNtYieldExecution)();

void MemoryPatcher::ScanPatch()
{
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    HMODULE ntdllMapped = LoadLibraryExA("ntdll.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);

    if (!ntdll || !ntdllMapped) return;

    try
    {
        LPCSTR funcName = "NtYieldExecution";

        auto hookedFunc = (PVOID)GetProcAddress(ntdll, funcName);

        DWORD64 moduleBase = (DWORD64)ntdll;
        auto funcData = RtlLookupFunctionEntry((DWORD64)hookedFunc, &moduleBase, nullptr);

        auto funcSize = funcData->EndAddress - funcData->BeginAddress;
        auto originalFunc = (PVOID)GetProcAddress(ntdllMapped, funcName);

        if (!hookedFunc || !originalFunc) return;

        auto result = RtlCompareMemory(hookedFunc, originalFunc, funcSize);

        if (result != funcSize)
        {
            DWORD oldprotect = 0;
            VirtualProtect(hookedFunc, funcSize, PAGE_EXECUTE_READWRITE, &oldprotect);
            RtlCopyMemory(hookedFunc, originalFunc, funcSize);

            result = RtlCompareMemory(hookedFunc, originalFunc, funcSize);
            if (result == funcSize)
            {
                VirtualProtect(hookedFunc, funcSize, oldprotect, &oldprotect);
            }
        }

        reinterpret_cast<pNtYieldExecution>(hookedFunc)();
    }
    catch (...)
    {
    }
}