#include "Memory.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <iostream>
DWORD Memory::GetProcId(const wchar_t* procName)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32W procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32FirstW(hSnap, &procEntry))
        {
            do
            {
                if (!_wcsicmp(procEntry.szExeFile, procName))
                {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

uintptr_t Memory::GetModuleBase(const char* module)
{
    uintptr_t moduleDLLBaseAddr = (uintptr_t)GetModuleHandleA(module);
    return moduleDLLBaseAddr;
}


