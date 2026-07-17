#include "Polymorphic.h"

uint8_t CPolymorphic::junkReg(CPolymorphic* o) 
{
    static const uint8_t r[] = { 0,1,2,3,6,7 };
    return r[o->randomize(0, 5)];
}


void CPolymorphic::appendJunk(CPolymorphic* o, std::vector<uint8_t>& out)
{
    switch (o->randomize(0, 5))
    {
    case 0: out.push_back(0x90); break;                                   // nop
    case 1: out.insert(out.end(), { 0x0F,0x1F,0x00 }); break;            // nop dword[rax]
    case 2: out.insert(out.end(), { 0x0F,0x1F,0x40,0x00 }); break;       // nop dword[rax+0]
    case 3: {                                                             // push r ; pop r
        uint8_t g = junkReg(o);
        out.push_back((uint8_t)(0x50 | g));
        out.push_back((uint8_t)(0x58 | g));
        break;
    }
    case 4: {                                                             // lea r,[r+0] (REX.W)
        uint8_t g = junkReg(o);
        out.insert(out.end(), { 0x48, 0x8D, (uint8_t)(0x40 | (g<<3) | g), 0x00 });
        break;
    }
    case 5: {                                                             // mov r,r (REX.W)
        uint8_t g = junkReg(o);
        out.insert(out.end(), { 0x48, 0x89, (uint8_t)(0xC0 | (g<<3) | g) });
        break;
    }
    }
}



uintptr_t CPolymorphic::RelocateWithJunk(uintptr_t start, size_t len)
{
    if (len == 0 || len > 0x4000)
        return 0;
    if (!IsRelocatable(start, len))
        return 0;                              // has relative/RIP operands -> refuse

    std::vector<uint8_t> buf;
    buf.reserve(len * 2 + 32);

    uintptr_t cur = start;
    uintptr_t end = start + len;

    while (cur < end)
    {
        hde64s hs;
        unsigned int ilen = hde64_disasm((void*)cur, &hs);
        if ((hs.flags & F_ERROR) || ilen == 0)
            return 0;                          // shouldn't happen (guard passed), be safe

        bool isRet = (hs.opcode == 0xC3 || hs.opcode == 0xC2 ||
            hs.opcode == 0xCB || hs.opcode == 0xCA);

        // Junk BEFORE each real instruction, but never before the terminating ret
        // (junk after ret would be dead). Randomize whether to insert for diversity.
        if (!isRet && this->randomize(0, 1) == 1)
            appendJunk(this, buf);

        for (unsigned int i = 0; i < ilen; ++i)
            buf.push_back(this->Read(cur + i));

        cur += ilen;
    }

    // Allocate fresh executable memory and publish the copy.
    void* mem = VirtualAlloc(nullptr, buf.size(),
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!mem)
        return 0;

    memcpy(mem, buf.data(), buf.size());
    FlushInstructionCache(GetCurrentProcess(), mem, buf.size());
    return (uintptr_t)mem;
}