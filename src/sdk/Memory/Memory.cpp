#include "Memory.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <iostream>

// process stuff
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



// memory editing
void Memory::Patch(BYTE* dst, BYTE* src, unsigned int size)
{
    // temporarily set readwrite perms
    DWORD oldprotect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);

    // copy into buffer
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldprotect, &oldprotect);
}

void Memory::Nop(BYTE* dst, unsigned int size)
{
    std::vector<BYTE> nopArray(size, NOP);
    Patch(dst, nopArray.data(), size);
}


PIMAGE_NT_HEADERS Memory::GetNTHeaders(PVOID module)
{
    if (!module)
        return nullptr;
    return (PIMAGE_NT_HEADERS)((PBYTE)module + PIMAGE_DOS_HEADER(module)->e_lfanew);
}