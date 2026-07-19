#include "../Polymorphic.h"
#include "Mutate.h"


#if defined(_M_X64) || defined(__x86_64__)
const uint8_t junk2[4][2] = { {0x8A, 0xC0}, {0x8A, 0xDB}, {0x8A, 0xC9}, {0x66, 0x90} };
const uint8_t junk3[4][3] = { {0x0F, 0x1F, 0xC0}, {0x0F, 0x1F, 0xC1}, {0x0F, 0x1F, 0xC2}, {0x0F, 0x1F, 0xC3} };
const uint8_t safe1ByteJunk[4] = { 0x90, 0xF8, 0xF9, 0xF5 }; // NOP, CLC, STC, CMC
#else
const uint8_t junk2[4][2] = { {0x8B, 0xC0}, {0x8B, 0xDB}, {0x87, 0xC9}, {0x66, 0x90} };
const uint8_t junk3[4][3] = { {0x8D, 0x76, 0x00}, {0x8D, 0x7F, 0x00}, {0x8D, 0x40, 0x00}, {0x0F, 0x1F, 0x00} };
const uint8_t safe1ByteJunk[4] = { 0x90, 0x98, 0x99, 0xF5}; // NOP, CBW, CWD, CMC
#endif



namespace Memory
{

	void Mutate1Byte(uintptr_t dwAddress, uint8_t b1)
    {
        if (b1 == 0xCC)
        {
            Write(dwAddress, safe1ByteJunk[randomize(0, 3)]);
        }
    }

    void Mutate2Byte(uintptr_t dwAddress, uint8_t b1)
    {
        CPolymorphic cp;
        CPolymorphic::Decoded d = cp.SkipRex(dwAddress, b1);

        uint8_t opcode = Read(d.opAddr);

        uint8_t b2  = Read(d.opAddr + 1);
        uint8_t mod = (b2 >> 6) & 0x03, reg = (b2 >> 3) & 0x07, rm = b2 & 0x07;
        if (mod != 0x03) return; // register-direct only

        uint8_t rexR = (d.rex >> 2) & 1, rexB = d.rex & 1;
        bool sameReg = (reg == rm) && (rexR == rexB);

        // same-register identity: TEST/OR/AND reg,reg
        if (sameReg && (opcode == 0x85 || opcode == 0x09 || opcode == 0x21)) 
        {
            const uint8_t t[] = { 0x85, 0x09, 0x21 };
            Write(d.opAddr, t[randomize(0, 2)]);
            cp.SetModRM(d.opAddr, rm, reg);
            return;
        }
        // same-register zeroing: XOR/SUB reg,reg
        if (sameReg && (opcode == 0x31 || opcode == 0x33 || opcode == 0x29 || opcode == 0x2B)) 
        {
            const uint8_t z[] = { 0x31, 0x33, 0x29, 0x2B };
            Write(d.opAddr, z[randomize(0, 3)]);
            cp.SetModRM(d.opAddr, rm, reg);
            return;
        }
        // MOV direction flip
        if (opcode == 0x89 || opcode == 0x8B) 
        {
            Write(d.opAddr, opcode ^ 0x02);
            cp.SetModRM(d.opAddr, rm, reg);
            cp.SwapRexRB(dwAddress, d.rex);
            return;
        }
        // ALU direction flip (bit 1 is the direction bit across the family)
        static const uint8_t alu[] = {0x01,0x03,0x09,0x0B,0x21,0x23,
            0x29,0x2B,0x31,0x33,0x39,0x3B};
        for (uint8_t x : alu) if (opcode == x) 
        {
            Write(d.opAddr, opcode ^ 0x02);
            cp.SetModRM(d.opAddr, rm, reg);
            cp.SwapRexRB(dwAddress, d.rex);
            return;
        }
    }

    void Mutate3Byte(uintptr_t dwAddress, uint8_t b1)      // 0x83: ADD/SUB imm8, CMP->TEST
    {
        CPolymorphic cp;
        CPolymorphic::Decoded d = cp.SkipRex(dwAddress, b1);
        if (Read(d.opAddr) != 0x83) return;

        uint8_t modrm = Read(d.opAddr + 1);
        uint8_t reg = (modrm >> 3) & 0x07, mod = (modrm >> 6) & 0x03, rm = modrm & 0x07;
        uint32_t immOff = cp.ImmOffset(d.opAddr);

        if (reg == 0 || reg == 5) 
        {                         // ADD <-> SUB
            uint8_t imm8 = Read(d.opAddr + immOff);

            if (imm8 == 0x80) return;                       // -128 doesn't round-trip

            cp.SetRegField(d.opAddr, modrm, reg == 0 ? 5 : 0);
            Write(d.opAddr + immOff, (uint8_t)(~imm8 + 1));
        }
        else if (reg == 7 && mod == 0x03) // CMP reg,0 -> TEST reg,reg 
        {                 
            if (Read(d.opAddr + immOff) != 0x00) return;
        
            Write(d.opAddr, 0x85);
            cp.SetModRM(d.opAddr, rm, rm);
            Write(d.opAddr + 2, 0x90); // freed imm byte -> NOP
        
            if (d.rex) // TEST needs REX.R == REX.B
            {                                    
                uint8_t b = d.rex & 1;
                Write(d.opAddr - 1, (uint8_t)((d.rex & 0xFB) | (b << 2)));
            }
        }
    }

    void Mutate5Byte(uintptr_t dwAddress, uint8_t b1) // 0x05/0x2D: ADD/SUB (e/r)AX, imm
    {
        CPolymorphic cp;
        CPolymorphic::Decoded d = cp.SkipRex(dwAddress, b1);
        uint8_t opcode = Read(d.opAddr);
        if (opcode != 0x05 && opcode != 0x2D) return;

        bool rexW; bool op16 = cp.OpSize16(d.opAddr, rexW);
        int  size = op16 ? 2 : 4;

        uint32_t imm = cp.ReadImm(d.opAddr + 1, size);
        if (rexW && size == 4 && imm == 0x80000000u) return; // no round-trip

        Write(d.opAddr, (opcode == 0x05) ? 0x2D : 0x05);
        cp.WriteImm(d.opAddr + 1, ~imm + 1, size);
    }

    void Mutate6Byte(uintptr_t dwAddress, uint8_t b1) // 0x81: ADD/SUB imm16/32
    {
        CPolymorphic cp;
        CPolymorphic::Decoded d = cp.SkipRex(dwAddress, b1);
        if (Read(d.opAddr) != 0x81) return;

        uint8_t modrm = Read(d.opAddr + 1);
        uint8_t reg = (modrm >> 3) & 0x07;
        if (reg != 0 && reg != 5) return;

        bool rexW; bool op16 = cp.OpSize16(d.opAddr, rexW);
        int  size = op16 ? 2 : 4;
        uint32_t immOff = cp.ImmOffset(d.opAddr);

        uint32_t imm = cp.ReadImm(d.opAddr + immOff, size);
        if (rexW && size == 4 && imm == 0x80000000u) return;

        cp.SetRegField(d.opAddr, modrm, reg == 0 ? 5 : 0);
        cp.WriteImm(d.opAddr + immOff, ~imm + 1, size);
    }

    void Mutate80(uintptr_t dwAddress, uint8_t b1)  // 0x80: ADD/SUB r/m8, imm8
    {
        CPolymorphic cp;
        CPolymorphic::Decoded d = cp.SkipRex(dwAddress, b1);
        if (Read(d.opAddr) != 0x80) return;

        uint8_t modrm = Read(d.opAddr + 1);
        uint8_t reg = (modrm >> 3) & 0x07;
        if (reg != 0 && reg != 5) return;

        uint32_t immOff = cp.ImmOffset(d.opAddr);
        uint8_t  imm8   = Read(d.opAddr + immOff);
        cp.SetRegField(d.opAddr, modrm, reg == 0 ? 5 : 0);
        Write(d.opAddr + immOff, (uint8_t)(~imm8 + 1)); // 8-bit: all values round-trip
    }

    void Mutate8BitRegToReg(uintptr_t dwAddress, uint8_t b1)
    {
        CPolymorphic cp;
        CPolymorphic::Decoded d = cp.SkipRex(dwAddress, b1);
        uint8_t opcode = Read(d.opAddr);

        uint8_t b2  = Read(d.opAddr + 1);
        uint8_t mod = (b2 >> 6) & 0x03, reg = (b2 >> 3) & 0x07, rm = b2 & 0x07;
        if (mod != 0x03) return;

        uint8_t rexR = (d.rex >> 2) & 1, rexB = d.rex & 1;
        bool sameReg = (reg == rm) && (rexR == rexB);

        if (sameReg && (opcode == 0x84 || opcode == 0x08 || opcode == 0x20)) 
        {
            const uint8_t t[] = { 0x84, 0x08, 0x20 };
            Write(d.opAddr, t[randomize(0, 2)]);
            cp.SetModRM(d.opAddr, rm, reg);
            return;
        }
        if (sameReg && (opcode == 0x30 || opcode == 0x32 || opcode == 0x28 || opcode == 0x2A)) 
        {
            const uint8_t z[] = { 0x30, 0x32, 0x28, 0x2A };
            Write(d.opAddr, z[randomize(0, 3)]);
            cp.SetModRM(d.opAddr, rm, reg);
            return;
        }
        if (opcode == 0x88 || opcode == 0x8A) 
        {
            Write(d.opAddr, opcode ^ 0x02);
            cp.SetModRM(d.opAddr, rm, reg);
            cp.SwapRexRB(dwAddress, d.rex);
            return;
        }
        static const uint8_t alu8[] = {0x00,0x02,0x08,0x0A,0x20,0x22,0x28,0x2A,0x30,0x32,0x38,0x3A};
        for (uint8_t x : alu8) if (opcode == x) 
        {
            Write(d.opAddr, opcode ^ 0x02);
            cp.SetModRM(d.opAddr, rm, reg);
            cp.SwapRexRB(dwAddress, d.rex);
            return;
        }
    }

    void Mutate8BitAccImm(uintptr_t dwAddress, uint8_t b1) 
    {
        CPolymorphic cp;
        CPolymorphic::Decoded d = cp.SkipRex(dwAddress, b1);
        uint8_t opcode = Read(d.opAddr);
        if (opcode != 0x04 && opcode != 0x2C) return;

        uint8_t imm8 = Read(d.opAddr + 1);
        Write(d.opAddr, (opcode == 0x04) ? 0x2C : 0x04);
        Write(d.opAddr + 1, (uint8_t)(~imm8 + 1));
    }

    void MutateTwoByte(uintptr_t dwAddress, bool pfx66, bool pfxF2, bool pfxF3)
    {
        uintptr_t opAddr = dwAddress;
        for (int i = 0; i < 4; ++i) 
        {                       
            uint8_t b = Read(opAddr);
            if (b==0x66||b==0xF2||b==0xF3||b==0x2E||b==0x36||b==0x3E||
                b==0x26||b==0x64||b==0x65||b==0x67) opAddr++;
            else break;
        }
    #if defined(_M_X64) || defined(__x86_64__)
        if ((Read(opAddr) & 0xF0) == 0x40) opAddr++;  
    #endif
        if (Read(opAddr) != 0x0F) return;
        if (pfxF2 || pfxF3) return;                          

        uint8_t op2 = Read(opAddr + 1);
        if (((Read(opAddr + 2) >> 6) & 0x03) != 0x03) return;   

        uint8_t nw;
        switch (op2)
        {                                       
        case 0x28: nw = 0x10; break;  case 0x10: nw = 0x28; break;
        case 0x29: nw = 0x11; break;  case 0x11: nw = 0x29; break;
        default: return;
        }
        Write(opAddr + 1, nw);
    }
};
