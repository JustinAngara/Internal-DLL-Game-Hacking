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
public:
    CPolymorphic(void);
    ~CPolymorphic(void);

    void Run(uintptr_t dwStart);
    uintptr_t RelocateWithJunk(uintptr_t start, size_t len); // Helper::SEH::__try_reloc needs this to be public
    // decode: skip a REX prefix, return opcode address + the REX byte
    struct Decoded
    {
        uintptr_t opAddr;   
        uint8_t   rex;      
    };

    Decoded SkipRex(uintptr_t dwAddress, uint8_t b1);
    void SetModRM(uintptr_t opAddr, uint8_t reg, uint8_t rm);
    
    // swap REX.R <-> REX.B (used whenever reg/rm fields are swapped)
    void SwapRexRB(uintptr_t rexAddr, uint8_t rex);
    
    // little-endian immediate read/write at a given width
    uint32_t ReadImm(uintptr_t a, int size);
    void WriteImm(uintptr_t a, uint32_t v, int size);
    
    // bytes from the opcode to the immediate 
    uint32_t ImmOffset(uintptr_t opAddr);
    
    // operand-size prefix
    bool OpSize16(uintptr_t opAddr, bool& rexW);
    
    // common field writes
    void SetRegField(uintptr_t opAddr, uint8_t modrm, uint8_t newReg);

private:

    unsigned char regs32[8];
    
    // mutate stuff
    struct Mutators;

    uint8_t JunkReg();    
    void AppendJunk(std::vector<uint8_t>& out);

    DWORD CalculateFunctionSize(DWORD_PTR dwStart);
    void ObfuscateOpcode(uintptr_t dwAddress);


    bool IsRelocatable(uintptr_t start, size_t len);

};

