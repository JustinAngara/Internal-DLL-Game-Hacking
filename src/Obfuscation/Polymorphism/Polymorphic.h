#pragma once

#include <Windows.h>

#include <ctime>
#include <cstdlib>

#include "ext/ade32/ADE32.h"

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

    bool PatchOpcode(DWORD dwAddress, BYTE bytes[], unsigned int bytecount);
    void ObfuscateOpcode(DWORD dwAddress, int opcodeLen);
    DWORD CalculateFunctionSize(DWORD dwStart);
public:
    CPolymorphic(void);
    ~CPolymorphic(void);

    void Run(DWORD dwStart);
};