#pragma once

#include <Windows.h>
#include <ctime>
#include <cstdlib>

#if defined(_M_IX86)
#include "ext/minhook/hde/hde32.h"
#elif defined(_M_X64) || defined(_M_AMD64)
#include "ext/minhook/hde/hde64.h"
#endif
class CPolymorphic
{
private:

    const uint8_t junk2[4][2] = {
        {0x8B, 0xC0}, {0x8B, 0xDB}, {0x87, 0xC9}, {0x66, 0x90}
    };

    const uint8_t junk3[4][3] = {
        {0x8D, 0x76, 0x00}, {0x8D, 0x7F, 0x00}, {0x8D, 0x40, 0x00}, {0x0F, 0x1F, 0x00}
    };

    const uint8_t safe1ByteJunk[4] = {
        0x90, 0x98, 0x99, 0xF5 // NOP, CBW, CWD, CMC
    };

    unsigned char regs32[8];

    inline unsigned char Read (DWORD dwAddress)
    {
        return *(unsigned char*)dwAddress;
    }
    inline void Write (DWORD dwAddress, unsigned char val)
    {
        *(unsigned char*)dwAddress = val;
    }
    inline int randomize(int min, int max)
    {
        return (1 + int((max - min + 1)*rand()/(RAND_MAX + 1.0)));
    }

    int oplen(BYTE* address) 
    {
#if defined(_M_IX86)
        hde32s hs;
        unsigned int len = hde32_disasm((void*)address, &hs);
        if (hs.flags & F_ERROR) return 0;
        return len;

#elif defined(_M_X64) || defined(_M_AMD64)
        hde64s hs;
        unsigned int len = hde64_disasm((void*)address, &hs);
        if (hs.flags & F_ERROR) return 0;
        return len;

#else
        return 0; 
#endif
    }


    DWORD CalculateFunctionSize(DWORD_PTR dwStart);
    bool PatchOpcode(DWORD dwAddress, BYTE bytes[], unsigned int bytecount);
    void ObfuscateOpcode(DWORD dwAddress, int opcodeLen);

    void Mutate1Byte(uint32_t dwAddress, uint8_t b1);
    void Mutate2Byte(uint32_t dwAddress, uint8_t b1);
    void Mutate3Byte(uint32_t dwAddress, uint8_t b1);
    void Mutate5Byte(uint32_t dwAddress, uint8_t b1);
    void Mutate6Byte(uint32_t dwAddress, uint8_t b1);

public:
    CPolymorphic(void);
    ~CPolymorphic(void);

    void Run(DWORD dwStart);
};