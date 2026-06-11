#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <vector>
namespace Memory
{
    // vars
    static constexpr int NOP = 0x90;

    // process helpers
    DWORD GetProcId(const wchar_t* procName);
    uintptr_t GetModuleBase(const char* module);

    // memory editing
    void Patch(BYTE* dest, BYTE* src, unsigned int size);
    void Nop(BYTE* dest, unsigned int size);
}