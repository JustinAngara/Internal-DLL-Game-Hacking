#pragma once
#include <cstdint>
#include <vector>
namespace PatternScan
{
    uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);
    std::vector<int> PatternToBytes(const char* pattern);
    uintptr_t Scan(const char* module, const char* signature);
}
