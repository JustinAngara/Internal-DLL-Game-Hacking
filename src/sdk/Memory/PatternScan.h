#pragma once
#include <cstdint>
#include <vector>
namespace Memory
{
    uintptr_t GetModuleBase(const char* module);
    
    uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);

    std::vector<int> PatternToBytes(const char* pattern);

    uintptr_t PatternScan(const char* module, const char* signature);
}
