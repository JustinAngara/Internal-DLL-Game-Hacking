#pragma once

#include <Windows.h>
#include <ctime>
#include <cstdlib>

#include "ext/minhook/hde/hde32.h"
#include "ext/minhook/hde/hde64.h"

class CPolymorphic
{
private:
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


public:
    CPolymorphic(void);
    ~CPolymorphic(void);

    void Run(DWORD dwStart);
};