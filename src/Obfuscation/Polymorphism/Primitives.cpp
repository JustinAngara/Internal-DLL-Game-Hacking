#include "Polymorphic.h"
#include "Mutators/Mutate.h"
#include <Windows.h>




CPolymorphic::Decoded CPolymorphic::SkipRex(uintptr_t dwAddress, uint8_t b1)
{
    Decoded d{ dwAddress, 0 };
#if defined(_M_X64) || defined(__x86_64__)
    if ((b1 & 0xF0) == 0x40) { d.rex = b1; d.opAddr = dwAddress + 1; }
#endif
    return d;
}



uint32_t CPolymorphic::ImmOffset(uintptr_t opAddr)
{
    uint8_t modrm = Memory::Read(opAddr + 1);
    uint8_t mod = (modrm >> 6) & 0x03;
    uint8_t rm  =  modrm       & 0x07;

    uint32_t off = 2;                       // opcode + ModR/M
    if (mod != 0x03) {
        if (rm == 0x04) {                   // SIB present
            uint8_t base = Memory::Read(opAddr + 2) & 0x07;
            off += 1;
            if (mod == 0x00 && base == 0x05) off += 4;
        }
        if      (mod == 0x01)               off += 1;   // disp8
        else if (mod == 0x02)               off += 4;   // disp32
        else if (mod == 0x00 && rm == 0x05) off += 4;   // RIP-relative
    }
    return off;
}



bool CPolymorphic::OpSize16(uintptr_t opAddr, bool& rexW)
{
    rexW = false;
    uint8_t prev = Memory::Read(opAddr - 1);
    if ((prev & 0xF0) == 0x40) { rexW = (prev & 0x08) != 0; prev = Memory::Read(opAddr - 2); }
    return prev == 0x66;
}


uint32_t CPolymorphic::ReadImm(uintptr_t a, int size)
{
    uint32_t v = 0;
    for (int i = 0; i < size; ++i) v |= (uint32_t)Memory::Read(a + i) << (i * 8);
    return v;
}


void CPolymorphic::WriteImm(uintptr_t a, uint32_t v, int size)
{
    for (int i = 0; i < size; ++i) Memory::Write(a + i, (uint8_t)(v >> (i * 8)));
}



// --- common field writes ---
void CPolymorphic::SetRegField(uintptr_t opAddr, uint8_t modrm, uint8_t newReg) 
{
    Memory::Write(opAddr + 1, (uint8_t)((modrm & 0xC7) | (newReg << 3)));
}
void CPolymorphic::SetModRM(uintptr_t opAddr, uint8_t reg, uint8_t rm) 
{
    Memory::Write(opAddr + 1, (uint8_t)(0xC0 | (reg << 3) | rm));
}

void CPolymorphic::SwapRexRB(uintptr_t rexAddr, uint8_t rex) 
{
#if defined(_M_X64) || defined(__x86_64__)
    if (rex) {
        uint8_t r = (rex >> 2) & 1, b = rex & 1;
        Memory::Write(rexAddr, (uint8_t)((rex & 0xFA) | (b << 2) | r));
    }
#endif
}
