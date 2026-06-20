#include "PatternScan.h"
#include <Windows.h>
#include <Psapi.h>
#include <vector>



uintptr_t PatternScan::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
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

// IDA style scan
// PatternScan::Scan("client.dll", "48 8B 05 ? ? ? ? 89 41 08");
uintptr_t PatternScan::Scan(const char* module, const char* signature)
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


std::vector<int> PatternScan::PatternToBytes(const char* pattern)
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


