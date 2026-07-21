#include "Polymorphic.h"
#include "Mutators/Mutate.h"
#include <iostream>

void CPolymorphic::ObfuscateOpcode(uintptr_t dwAddress)
{
    uintptr_t a = dwAddress;

    bool pfx66 = false, pfxF2 = false, pfxF3 = false;

    // --- peel legacy prefixes (bounded; real instrs have <=4) ---
    for (int i = 0; i < 4; ++i) {
        uint8_t b = Memory::Read(a);
        if      (b == 0x66) { pfx66 = true; a++; }
        else if (b == 0xF2) { pfxF2 = true; a++; }
        else if (b == 0xF3) { pfxF3 = true; a++; }
        else if (b == 0x2E || b == 0x36 || b == 0x3E ||    // segment overrides
            b == 0x26 || b == 0x64 || b == 0x65 ||
            b == 0x67){ a++; }        // addr-size
        else break;
    }

    // --- REX must be the LAST prefix, immediately before the opcode ---
    uint8_t rex = 0;
#if defined(_M_X64) || defined(__x86_64__)
    {
        uint8_t b = Memory::Read(a);
        if ((b & 0xF0) == 0x40) { rex = b; a++; }
    }
#endif

    uint8_t op = Memory::Read(a);
    // `a` now points AT the opcode; hand that to the handlers so their
    // internal "skip REX" logic still works (pass dwAddress, they re-find it).

    // ---- two-byte (0F) opcode space ----
    if (op == 0x0F) {
        Memory::MutateTwoByte(dwAddress, pfx66, pfxF2, pfxF3);
        return;
    }

    // ---- one-byte space ----
    switch (op)
    {
    
        // 32/16-bit reg-to-reg ALU + MOV + TEST  (pfx66 -> 16-bit, same opcodes)
    case 0x89: case 0x8B:
    case 0x01: case 0x03: case 0x09: case 0x0B:
    case 0x21: case 0x23: case 0x29: case 0x2B:
    case 0x31: case 0x33: case 0x39: case 0x3B:
    case 0x85: Memory::Mutate2Byte(dwAddress, Memory::Read(dwAddress));  break;

     // 8-bit reg-to-reg ALU + MOV + TEST
    case 0x00: case 0x02: case 0x08: case 0x0A:
    case 0x20: case 0x22: case 0x28: case 0x2A:
    case 0x30: case 0x32: case 0x38: case 0x3A:
    case 0x84: case 0x88: case 0x8A: Memory::Mutate8BitRegToReg(dwAddress, Memory::Read(dwAddress)); break;
    case 0x80: Memory::Mutate80(dwAddress,    Memory::Read(dwAddress));                 break;
    case 0x81: Memory::Mutate6Byte(dwAddress, Memory::Read(dwAddress));                 break;
    case 0x83: Memory::Mutate3Byte(dwAddress, Memory::Read(dwAddress));                 break;
    case 0x05: case 0x2D: Memory::Mutate5Byte(dwAddress, Memory::Read(dwAddress));      break;
    case 0x04: case 0x2C: Memory::Mutate8BitAccImm(dwAddress, Memory::Read(dwAddress)); break;
    case 0xCC: Memory::Mutate1Byte(dwAddress, Memory::Read(dwAddress));                 break;
    
    default: break;
    }
}



bool CPolymorphic::IsRelocatable(uintptr_t start, size_t len)
{
    uintptr_t cur = start;
    uintptr_t end = start + len;

    while (cur < end)
    {
#ifdef _WIN64
        hde64s hs;
        unsigned int ilen = hde64_disasm((void*)cur, &hs);
#else
        hde32s hs;
        unsigned int ilen = hde32_disasm((void*)cur, &hs);
#endif
        if ((hs.flags & F_ERROR) || ilen == 0)
            return false;                      // can't decode -> refuse

        uint8_t op = hs.opcode;

        // Relative branches (one-byte space):
        if (op == 0xE8 || op == 0xE9 || op == 0xEB ||        // call/jmp rel
            (op >= 0x70 && op <= 0x7F) ||                    // jcc rel8
            (op >= 0xE0 && op <= 0xE3))                      // loop*/jrcxz
            return false;

        // Relative branches (two-byte space): 0F 80..8F jcc rel32
        if (op == 0x0F && hs.opcode2 >= 0x80 && hs.opcode2 <= 0x8F)
            return false;

        // RIP-relative memory operand: modrm mod=00, rm=101 (64-bit).
        if ((hs.flags & F_MODRM) && hs.modrm_mod == 0x00 && hs.modrm_rm == 0x05)
            return false;

        cur += ilen;
    }
    return true;
}




void CPolymorphic::Run(uintptr_t dwStart)
{
    DWORD dwLength = CalculateFunctionSize(dwStart);
    if (dwLength == 0 || dwLength > 0x4000)
    {
        return;
    }

    uintptr_t dwCurrent = dwStart;
    uintptr_t hardEnd   = dwStart + dwLength;

    // harness is responsible for making [dwStart, hardEnd) writable
    while (dwCurrent < hardEnd)
    {
        int nOpcodeLen = Memory::oplen((BYTE*)dwCurrent);

        // desync -> stop
        if (nOpcodeLen <= 0) break;

        // spill -> stop
        if (dwCurrent + nOpcodeLen > hardEnd) break;   

        this->ObfuscateOpcode(dwCurrent);
        dwCurrent += nOpcodeLen;
    }
}


CPolymorphic::CPolymorphic(void)
{
    srand((unsigned)time(0));

    this->regs32[0] = 0xC0;
    this->regs32[1] = 0xDB;
    this->regs32[2] = 0xC9;
    this->regs32[3] = 0xD2;
    this->regs32[4] = 0xE4;
    this->regs32[5] = 0xED;
    this->regs32[6] = 0xF6;
    this->regs32[7] = 0xFF;

}

CPolymorphic::~CPolymorphic(void)
{
}




/////////////////////////////////DEPENDENT BIT OPERATION//////////////////////////

DWORD CPolymorphic::CalculateFunctionSize(DWORD_PTR dwStart) 
{
#if defined(_M_IX86)
    // 32bit x86 implementation

    DWORD dwMaxBytes = 4096;
    DWORD_PTR dwCurrent = dwStart;
    DWORD dwLength = 0;
    DWORD dwLastRetOffset = 0;
    bool bFoundRet = false;

    while (dwLength < dwMaxBytes) 
    {
        BYTE currentByte = *(BYTE*)dwCurrent;

        bool isRet = (currentByte == 0xC3 ||  // near RET
            currentByte == 0xC2 ||  // near RET imm16
            currentByte == 0xCB ||  // far RETF
            currentByte == 0xCA);   // far RETF imm16

        int instrLen = Memory::oplen((BYTE*)dwCurrent);

        if (instrLen == 0) break; // unknown instruction, bail

        dwLength += instrLen;
        dwCurrent += instrLen;

        if (isRet) 
        {
            dwLastRetOffset = dwLength;
            bFoundRet = true;

            // if next byte looks like a new function prologue, stop
            BYTE nextByte = *(BYTE*)dwCurrent;
            if (nextByte == 0x55 || nextByte == 0xCC) // PUSH EBP or INT3 padding
            {
                break;
            }
        }
    }

    // return length through the last RET we found, not just the first
    return bFoundRet ? dwLastRetOffset : dwLength;

#elif defined(_M_X64) || defined(_M_AMD64)
    DWORD dwMaxBytes = 4096;
    DWORD_PTR dwCurrent = dwStart;
    DWORD dwLength = 0;
    DWORD dwLastRetOffset = 0;
    bool bFoundRet = false;

    while (dwLength < dwMaxBytes) 
    {
        BYTE currentByte = *(BYTE*)dwCurrent;
        bool isRet = (currentByte == 0xC3 || 0xC2 || 0xCB || 0xCA);

        // use HDE64 directly to get the length of the x64 instruction
        hde64s hs;
        int instrLen = hde64_disasm((void*)dwCurrent, &hs);

        // if HDE64 hits an error parsing the instruction, bail out
        if ((hs.flags & F_ERROR) || instrLen == 0) break; 

        dwLength += instrLen;
        dwCurrent += instrLen;

        if (isRet) 
        {
            dwLastRetOffset = dwLength;
            bFoundRet = true;

            // on x64, we drop the 0x55 check and only look for CC padding
            BYTE nextByte = *(BYTE*)dwCurrent;
            if (nextByte == 0xCC) // INT3 padding
            {
                break;
            }
        }
    }

    return bFoundRet ? dwLastRetOffset : dwLength;

#else
    return 0;
#endif
}