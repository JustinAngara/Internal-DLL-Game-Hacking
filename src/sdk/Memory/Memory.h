#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <vector>
namespace Memory
{
    // process
    DWORD GetProcId(const wchar_t* procName);
    uintptr_t GetModuleBase(const char* module);
}