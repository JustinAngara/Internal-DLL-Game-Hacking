#include "Polymorphic.h"
#include <iostream>

void CPolymorphic::ObfuscateOpcode(DWORD dwAddress, int opcodeLen)
{
    uint8_t b1 = this->Read(dwAddress);

    switch (opcodeLen)
    {
    case 1: this->Mutate1Byte(dwAddress, b1); break;
    case 2: this->Mutate2Byte(dwAddress, b1); break;
    case 3: this->Mutate3Byte(dwAddress, b1); break;
    case 5: this->Mutate5Byte(dwAddress, b1); break;
    case 6: this->Mutate6Byte(dwAddress, b1); break;
    default: break;
    }
}


void CPolymorphic::Mutate1Byte(uintptr_t dwAddress, uint8_t b1)
{
    if (b1 == 0xCC) 
    {
        this->Write(dwAddress, safe1ByteJunk[this->randomize(0, 3)]);
    }
}

void CPolymorphic::Mutate2Byte(uintptr_t dwAddress, uint8_t b1)
{
    uint8_t opcode = b1;
    uintptr_t opAddr = dwAddress;
    uint8_t rex = 0;

#if defined(_M_X64) || defined(__x86_64__)
    // Detect and preserve REX prefix (0x40 - 0x4F)
    if ((b1 & 0xF0) == 0x40) 
    {
        rex = b1;
        opcode = this->Read(dwAddress + 1);
        opAddr = dwAddress + 1;
    }
#endif

    uint8_t b2 = this->Read(opAddr + 1);
    bool isRegToReg = (b2 & 0xC0) == 0xC0; // Mod == 11
    uint8_t reg = (b2 >> 3) & 0x07;
    uint8_t rm = b2 & 0x07;
    bool isSameReg = (reg == rm);

#if !defined(_M_X64) && !defined(__x86_64__)
    // PUSH/POP <-> MOV (x86 ONLY)
    // Disabled in x64: PUSH/POP r64 is 2 bytes, but MOV r64 requires a 3-byte REX prefix.
    // In-place mutation is impossible here due to size mismatch.
    if ((opcode & 0xF8) == 0x50 && (b2 & 0xF8) == 0x58) 
    {
        uint8_t src = opcode & 0x07;
        uint8_t dst = b2 & 0x07;
        this->Write(dwAddress, 0x8B); 
        this->Write(dwAddress + 1, 0xC0 | (dst << 3) | src);
    }
    else 
#endif
        if (isRegToReg)
        {
            // DIRECTION BIT FLIPPER & MOV PROXY
            if (opcode == 0x8B || opcode == 0x89) 
            {
#if !defined(_M_X64) && !defined(__x86_64__)
                if (this->randomize(0, 1) == 1 && reg != 4 && rm != 4) 
                {
                    uint8_t src = (opcode == 0x8B) ? rm : reg;
                    uint8_t dst = (opcode == 0x8B) ? reg : rm;
                    this->Write(dwAddress, 0x50 | src); // PUSH src
                    this->Write(dwAddress + 1, 0x58 | dst); // POP dst
                }
                else
#endif
                {
                    this->Write(opAddr, (opcode == 0x8B) ? 0x89 : 0x8B);
                    this->Write(opAddr + 1, 0xC0 | (rm << 3) | reg);

#if defined(_M_X64) || defined(__x86_64__)
                    // If swapping reg/rm in x64, we MUST swap REX.R and REX.B bits
                    if (rex) 
                    {
                        uint8_t rexR = (rex & 0x04) >> 2;
                        uint8_t rexB = (rex & 0x01);
                        this->Write(dwAddress, (rex & 0xFA) | (rexB << 2) | rexR);
                    }
#endif
                }
            }
            // ALU DIRECTION BIT FLIPPER
            else
            {
                const uint8_t aluPairs[][2] = {
                    {0x03, 0x01}, {0x0B, 0x09}, {0x23, 0x21}, 
                    {0x2B, 0x29}, {0x33, 0x31}, {0x3B, 0x39} 
                };

                for (int i = 0; i < 6; i++) {
                    if (opcode == aluPairs[i][0] || opcode == aluPairs[i][1]) {
                        uint8_t new_opcode = (opcode == aluPairs[i][0]) ? aluPairs[i][1] : aluPairs[i][0];
                        this->Write(opAddr, new_opcode);
                        this->Write(opAddr + 1, 0xC0 | (rm << 3) | reg);

#if defined(_M_X64) || defined(__x86_64__)
                        if (rex) 
                        {
                            uint8_t rexR = (rex & 0x04) >> 2;
                            uint8_t rexB = (rex & 0x01);
                            this->Write(dwAddress, (rex & 0xFA) | (rexB << 2) | rexR);
                        }
#endif
                        break;
                    }
                }
            }

            // LOGIC IDENTITY TRIAD & ZEROING
            if (isSameReg) 
            {
                if (opcode == 0x85 || opcode == 0x09 || opcode == 0x21) 
                {
                    uint8_t equivalents[] = {0x85, 0x09, 0x21};
                    this->Write(opAddr, equivalents[this->randomize(0, 2)]);
                }
                else if (opcode == 0x31 || opcode == 0x33 || opcode == 0x29 || opcode == 0x2B) 
                {
                    uint8_t zeroes[] = {0x31, 0x33, 0x29, 0x2B};
                    this->Write(opAddr, zeroes[this->randomize(0, 3)]);
                }
            }
        }
}


void CPolymorphic::Mutate3Byte(uintptr_t dwAddress, uint8_t b1)
{
    uint8_t opcode = b1;
    uintptr_t opAddr = dwAddress;
    uint8_t rex = 0;

#if defined(_M_X64) || defined(__x86_64__)
    if ((b1 & 0xF0) == 0x40) 
    {
        rex = b1;
        opcode = this->Read(dwAddress + 1);
        opAddr = dwAddress + 1;
    }
#endif

    uint8_t b2 = this->Read(opAddr + 1);
    uint8_t b3 = this->Read(opAddr + 2);

    if (opcode == 0x83) 
    {
        uint8_t op = (b2 >> 3) & 0x07;
        uint8_t rm = b2 & 0x07;

        if (op == 0) // ADD
        {
            this->Write(opAddr + 1, (b2 & 0xC7) | (5 << 3)); // change to SUB
            this->Write(opAddr + 2, (uint8_t)(~b3 + 1));     // negate imm8
        }
        else if (op == 5) // SUB
        {
            this->Write(opAddr + 1, (b2 & 0xC7) | (0 << 3)); // change to ADD
            this->Write(opAddr + 2, (uint8_t)(~b3 + 1));     // negate imm8
        }
        else if (op == 7 && b3 == 0x00 && (b2 & 0xC0) == 0xC0) // CMP reg, 0
        {
            this->Write(opAddr, 0x85); // TEST
            this->Write(opAddr + 1, 0xC0 | (rm << 3) | rm);
            this->Write(opAddr + 2, 0x90);

#if defined(_M_X64) || defined(__x86_64__)
            if (rex) 
            {
                uint8_t rexB = (rex & 0x01);
                this->Write(dwAddress, (rex & 0xFB) | (rexB << 2)); // copy REX.B into REX.R
            }
#endif
        }
    }
}

void CPolymorphic::Mutate5Byte(uintptr_t dwAddress, uint8_t b1){
    uint8_t opcode = b1;
    uintptr_t opAddr = dwAddress;

#if defined(_M_X64) || defined(__x86_64__)
    if ((b1 & 0xF0) == 0x40) 
    {
        opcode = this->Read(dwAddress + 1);
        opAddr = dwAddress + 1;
    }
#endif

    uint8_t b2 = this->Read(opAddr + 1);

    if (opcode == 0x05 || opcode == 0x2D) // ADD/SUB EAX, imm32
    {
        uint8_t b3 = this->Read(opAddr + 2);
        uint8_t b4 = this->Read(opAddr + 3);
        uint8_t b5 = this->Read(opAddr + 4);

        uint32_t imm = (b5 << 24) | (b4 << 16) | (b3 << 8) | b2;
        uint32_t negImm = ~imm + 1;

        this->Write(opAddr, (opcode == 0x05) ? 0x2D : 0x05);
        this->Write(opAddr + 1, negImm & 0xFF);
        this->Write(opAddr + 2, (negImm >> 8) & 0xFF);
        this->Write(opAddr + 3, (negImm >> 16) & 0xFF);
        this->Write(opAddr + 4, (negImm >> 24) & 0xFF);
    }
}




void CPolymorphic::Mutate6Byte(uintptr_t dwAddress, uint8_t b1)
{
    uint8_t opcode = b1;
    uintptr_t opAddr = dwAddress;

#if defined(_M_X64) || defined(__x86_64__)
    if ((b1 & 0xF0) == 0x40) 
    {
        opcode = this->Read(dwAddress + 1);
        opAddr = dwAddress + 1;
    }
#endif

    if (opcode == 0x81) 
    {
        uint8_t b2 = this->Read(opAddr + 1);
        uint8_t op = (b2 >> 3) & 0x07;

        if (op == 0 || op == 5) 
        {
            uint8_t b3 = this->Read(opAddr + 2);
            uint8_t b4 = this->Read(opAddr + 3);
            uint8_t b5 = this->Read(opAddr + 4);
            uint8_t b6 = this->Read(opAddr + 5);

            uint32_t imm = (b6 << 24) | (b5 << 16) | (b4 << 8) | b3;
            uint32_t negImm = ~imm + 1;

            this->Write(opAddr + 1, (op == 0) ? ((b2 & 0xC7) | (5 << 3)) : ((b2 & 0xC7) | (0 << 3)));
            this->Write(opAddr + 2, negImm & 0xFF);
            this->Write(opAddr + 3, (negImm >> 8) & 0xFF);
            this->Write(opAddr + 4, (negImm >> 16) & 0xFF);
            this->Write(opAddr + 5, (negImm >> 24) & 0xFF);
        }
    }
}

void CPolymorphic::Run(DWORD dwStart)
{
    DWORD dwLength = CalculateFunctionSize(dwStart);
    DWORD dwCurrent = dwStart;
    MEMORY_BASIC_INFORMATION mbi;

    while (true)
    {
        VirtualQuery((void*)dwCurrent, &mbi, sizeof(mbi));

        // Check if the memory page is mapped and accessible
        if (mbi.State == MEM_COMMIT && mbi.Protect != 0x1)
        {
            DWORD prevAccess, newAccess, startAddress(dwCurrent);

            // Enable writing access to the code
            VirtualProtect((PBYTE)dwCurrent, ((DWORD)mbi.BaseAddress + mbi.RegionSize) - startAddress, PAGE_EXECUTE_READWRITE, &prevAccess);

            while ((dwCurrent < (DWORD)mbi.BaseAddress + static_cast<DWORD>(mbi.RegionSize)) && (dwCurrent < dwStart + dwLength))
            {
                int nOpcodeLen = oplen((BYTE*)dwCurrent);

                if (nOpcodeLen > 0)
                {
                    this->ObfuscateOpcode(dwCurrent, nOpcodeLen);
                    dwCurrent += nOpcodeLen;
                }
                else
                    dwCurrent++;
            }

            // Apply old access to the code
            VirtualProtect((PBYTE)startAddress, ((DWORD)mbi.BaseAddress + mbi.RegionSize) - startAddress, prevAccess, &newAccess);
        }
        else
            dwCurrent = (DWORD)mbi.BaseAddress + mbi.RegionSize + 0x1; // Go to the next region

        if (dwCurrent >= dwStart + dwLength)
            break;
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

DWORD CPolymorphic::CalculateFunctionSize(DWORD_PTR dwStart) {
#if defined(_M_IX86)
    // 32bit x86 implementation

    DWORD dwMaxBytes = 4096;
    DWORD_PTR dwCurrent = dwStart;
    DWORD dwLength = 0;
    DWORD dwLastRetOffset = 0;
    bool bFoundRet = false;

    while (dwLength < dwMaxBytes) {
        BYTE currentByte = *(BYTE*)dwCurrent;

        bool isRet = (currentByte == 0xC3 ||  // near RET
            currentByte == 0xC2 ||  // near RET imm16
            currentByte == 0xCB ||  // far RETF
            currentByte == 0xCA);   // far RETF imm16

        int instrLen = oplen((BYTE*)dwCurrent);

        if (instrLen == 0) break; // unknown instruction, bail

        dwLength += instrLen;
        dwCurrent += instrLen;

        if (isRet) {
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

    while (dwLength < dwMaxBytes) {
        BYTE currentByte = *(BYTE*)dwCurrent;

        bool isRet = (currentByte == 0xC3 ||  // near RET
            currentByte == 0xC2 ||  // near RET imm16
            currentByte == 0xCB ||  // far RETF
            currentByte == 0xCA);   // far RETF imm16

        // use HDE64 directly to get the length of the x64 instruction
        hde64s hs;
        int instrLen = hde64_disasm((void*)dwCurrent, &hs);

        // if HDE64 hits an error parsing the instruction, bail out
        if ((hs.flags & F_ERROR) || instrLen == 0) break; 

        dwLength += instrLen;
        dwCurrent += instrLen;

        if (isRet) {
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
#error "CPolymorphic engine only supports x86 and x64 architectures."
    return 0;
#endif
}