#pragma once

#include <Windows.h>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <cstdint>

#if defined(_M_IX86)
#include "ext/minhook/hde/hde32.h"
#elif defined(_M_X64) || defined(_M_AMD64)
#include "ext/minhook/hde/hde64.h"
#endif
class CPolymorphic
{
private:

#if defined(_M_X64) || defined(__x86_64__)
    const uint8_t junk2[4][2] = { {0x8A, 0xC0}, {0x8A, 0xDB}, {0x8A, 0xC9}, {0x66, 0x90} };
    const uint8_t junk3[4][3] = { {0x0F, 0x1F, 0xC0}, {0x0F, 0x1F, 0xC1}, {0x0F, 0x1F, 0xC2}, {0x0F, 0x1F, 0xC3} };
    const uint8_t safe1ByteJunk[4] = { 0x90, 0xF8, 0xF9, 0xF5 }; // NOP, CLC, STC, CMC
#else
    const uint8_t junk2[4][2] = { {0x8B, 0xC0}, {0x8B, 0xDB}, {0x87, 0xC9}, {0x66, 0x90} };
    const uint8_t junk3[4][3] = { {0x8D, 0x76, 0x00}, {0x8D, 0x7F, 0x00}, {0x8D, 0x40, 0x00}, {0x0F, 0x1F, 0x00} };
    const uint8_t safe1ByteJunk[4] = { 0x90, 0x98, 0x99, 0xF5}; // NOP, CBW, CWD, CMC
#endif



    unsigned char regs32[8];

    inline unsigned char Read (uintptr_t dwAddress)
    {
        return *(unsigned char*)dwAddress;
    }
    inline void Write (uintptr_t dwAddress, unsigned char val)
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

    uint8_t junkReg(CPolymorphic* o);    
    void appendJunk(CPolymorphic* o, std::vector<uint8_t>& out);

    // decode: skip a REX prefix, return opcode address + the REX byte
    struct Decoded
    {
        uintptr_t opAddr;   
        uint8_t   rex;      
    };

    Decoded SkipRex(uintptr_t dwAddress, uint8_t b1);

    // bytes from the opcode to the immediate 
    uint32_t ImmOffset(uintptr_t opAddr);

    // operand-size prefix
    bool OpSize16(uintptr_t opAddr, bool& rexW);

    // little-endian immediate read/write at a given width
    uint32_t ReadImm(uintptr_t a, int size);
    void WriteImm(uintptr_t a, uint32_t v, int size);


    // common field writes
    void SetRegField(uintptr_t opAddr, uint8_t modrm, uint8_t newReg);
    void SetModRM(uintptr_t opAddr, uint8_t reg, uint8_t rm);

    // swap REX.R <-> REX.B (used whenever reg/rm fields are swapped)
    void SwapRexRB(uintptr_t rexAddr, uint8_t rex);


    DWORD CalculateFunctionSize(DWORD_PTR dwStart);
    void ObfuscateOpcode(uintptr_t dwAddress);

    // mutate stuff
    void Mutate1Byte(uintptr_t dwAddress, uint8_t b1);
    void Mutate2Byte(uintptr_t dwAddress, uint8_t b1);
    void Mutate3Byte(uintptr_t dwAddress, uint8_t b1);
    void Mutate5Byte(uintptr_t dwAddress, uint8_t b1);
    void Mutate6Byte(uintptr_t dwAddress, uint8_t b1);
    void MutateTwoByte(uintptr_t dwAddress, bool pfx66, bool pfxF2, bool pfxF3);
    void Mutate80(uintptr_t dwAddress, uint8_t b1);
    void Mutate8BitRegToReg(uintptr_t dwAddress, uint8_t b1);
    void Mutate8BitAccImm(uintptr_t dwAddress, uint8_t b1);
    bool IsRelocatable(uintptr_t start, size_t len);


public:
    CPolymorphic(void);
    ~CPolymorphic(void);

    void Run(uintptr_t dwStart);
    uintptr_t RelocateWithJunk(uintptr_t start, size_t len); // Helper::SEH::__try_reloc needs this to be public

};

