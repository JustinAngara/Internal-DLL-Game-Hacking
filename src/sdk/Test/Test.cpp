#include <cstdint>
#include <cstddef>
#include <cstdio>

#include "Obfuscation/Polymorphism/Polymorphic.h"

#include <Windows.h>   
#include <cstdio>      
#include <cstring>     

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

volatile uint64_t g_sink = 0;

// ====================== target functions ======================

NOINLINE void foo_A() {
    __asm { nop };
    __asm { nop };
    uint32_t x = 0x1337;
    __asm { mov eax, eax };   // 8B C0 — engine will mutate this
    __asm { nop };
    __asm { nop };
    for (uint32_t i = 0; i < 7; ++i) x = (x << 3) ^ (x + i);
    g_sink += x;
}

NOINLINE void bar_A() {
    __asm { mov ebx, ebx };   // 8B DB
    uint64_t a = 0xDEADBEEF, b = 0xCAFEBABE;
    __asm { nop };
    __asm { nop };
    __asm { mov ecx, ecx };   // 8B C9
    a = (a * 31) ^ (b >> 5);
    b ^= (a << 11) | 0xF0;
    g_sink += a ^ b;
}

NOINLINE void foo_B() {
    __asm { mov edx, edx };   // 8B D2
    __asm { nop };
    __asm { nop };
    uint8_t buf[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
    uint32_t acc = 0;
    __asm { mov esi, esi };   // 8B F6
    for (int i = 0; i < 4; ++i) acc = (acc << 8) | (buf[i] + i);
    g_sink += acc;
}

NOINLINE void bar_B() {
    __asm { mov edi, edi };   // 8B FF
    __asm { nop };
    __asm { nop };
    int32_t s = 0;
    __asm { mov ebp, ebp };   // 8B ED
    for (int32_t i = 1; i <= 13; ++i) s += (i * i) - (i >> 1);
    g_sink += (uint64_t)s;
}

// ====================== hashing ===============================

uint64_t fnv1a(const uint8_t* data, size_t len) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; ++i) {
        h ^= data[i];
        h *= 1099511628211ULL;
    }
    return h;
}

size_t func_len(const void* fn, size_t cap = 512) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(fn);
    for (size_t i = 0; i < cap; ++i)
        if (p[i] == 0xC3) return i + 1;
    return cap;
}

uint64_t hash_func(const void* fn) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(fn);
    size_t len = func_len(fn);

    uint64_t h = 0;
    for (size_t i = 0; i < len; ++i)
        h += (uint64_t)p[i] * (i + 1);   // position-weighted: byte at index 0
    // contributes differently to index 1

    return h;
}

// ====================== driver ================================

namespace Test { namespace Obfuscation {

    struct Entry {
        const char* name;
        void (*fn)();
    };

    // table lives here at namespace scope — visible to Run()
    static Entry table[] = {
        { "foo_A", &foo_A },
        { "bar_A", &bar_A },
        { "foo_B", &foo_B },
        { "bar_B", &bar_B },
    };

    
    void Run() {
        char dllPath[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            (LPCSTR)&Run,
            &hSelf
        );
        GetModuleFileNameA(hSelf, dllPath, MAX_PATH);
        char* slash = strrchr(dllPath, '\\');
        if (slash) *(slash + 1) = '\0';
        strcat_s(dllPath, MAX_PATH, "signatures.txt");

        FILE* f = fopen(dllPath, "w");
        if (!f) return;

        fprintf(f, "%-6s  %-10s  %-6s  first bytes\n", "func", "address", "len");
        fprintf(f, "------  ----------  ------  ------------------------\n");

        CPolymorphic poly;

        const int count = sizeof(table) / sizeof(table[0]);
        for (int i = 0; i < count; ++i) {

            // resolve address
            void* addr = (void*)(DWORD)table[i].fn;
            uint8_t* p = (uint8_t*)addr;

            // follow E9 JMP thunk (MSVC debug builds)
            if (p[0] == 0xE9) {
                DWORD rel = *(DWORD*)(p + 1);
                addr = (void*)((DWORD)p + 5 + rel);
                p = (uint8_t*)addr;
            }

            // dump first 8 bytes so we can confirm we're at a real prologue (55 8B EC)
            fprintf(f, "%-6s  %p  ", table[i].name, addr);
            for (int b = 0; b < 8; ++b)
                fprintf(f, "%02X ", p[b]);
            fprintf(f, "\n");

            size_t   len    = func_len(addr);
            uint64_t before = hash_func(addr);

            // mutate
            DWORD old;
            VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &old);
            poly.Run((DWORD)addr);
            VirtualProtect(addr, len, old, &old);

            uint64_t after = hash_func(addr);

            fprintf(f, "       len=%-4zu  before=%016llx  after=%016llx  %s\n\n",
                len,
                (unsigned long long)before,
                (unsigned long long)after,
                before == after ? "SAME" : "DIFF");
        }

        fclose(f);
    }

}} // namespace Test::Obfuscation

int main() {
    Test::Obfuscation::Run();
    return 0;
}