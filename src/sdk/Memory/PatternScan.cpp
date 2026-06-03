#include "PatternScan.h"
#include <Windows.h>
#include <Psapi.h>
#include <vector>

uintptr_t Memory::GetModuleBase(const char* module)
{
    uintptr_t moduleDLLBaseAddr = (uintptr_t)GetModuleHandle(module);
    return moduleDLLBaseAddr;
}


uintptr_t Memory::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
{
    uintptr_t addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        if (!addr) return 0;          
        addr = *(uintptr_t*)addr;
        addr += offsets[i];
    }
    return addr;
}


std::vector<int> Memory::PatternToBytes(const char* pattern)
{
    std::vector<int> bytes;
    const char* c = pattern;
    while (*c)
    {
        if (*c == ' ') { ++c; continue; }
        if (*c == '?') {
            bytes.push_back(-1);
            while (*c == '?') ++c;
        } else {
            bytes.push_back(static_cast<int>(strtoul(c, const_cast<char**>(&c), 16)));
        }
    }
    return bytes;
}


uintptr_t Memory::PatternScan(const char* module, const char* signature)
{
    HMODULE mod = GetModuleHandleA(module);
    if (!mod) return 0;

    MODULEINFO info{};
    if (!GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(info)))
        return 0;

    auto base = reinterpret_cast<uint8_t*>(mod);
    auto size = info.SizeOfImage;

    auto pattern = PatternToBytes(signature);
    auto data = pattern.data();
    auto len = pattern.size();

    for (size_t i = 0; i < size - len; ++i)
    {
        bool found = true;
        for (size_t j = 0; j < len; ++j)
        {
            if (data[j] != -1 && data[j] != base[i + j])
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return reinterpret_cast<uintptr_t>(base + i);
        }
    }

    return 0;
}
