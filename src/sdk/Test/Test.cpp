#include <cstdint>
#include <cstdio>

#include "Obfuscation/Polymorphism/Polymorphic.h"
#include "sdk/Math/Math.h"

#include <Windows.h>   
#include <cstring>     



int foo1(int a, int b) {
    int c = a ^ b;
    for (int i = 0; i < (c > 0 ? c % 10 : -c % 10); i++) {
        c += (a * b) - i;
    }
    return c;
}

void foo2(char* p) {
    if (!p) return;
    while (*p) {
        *p = *p ^ 0x55;
        *p = *p ^ 0x55;
        p++;
    }
}

unsigned int foo3(unsigned int x) {
    x = (x >> 7) | (x << 25);
    x ^= 0xCAFEBABE;
    return x - 0x0BADC0DE;
}

float foo4(float f, int limit) {
    float res = f;
    if (limit > 100) {
        res *= 1.5f;
    } else if (limit < 0) {
        res -= 3.14f;
    } else {
        res += (float)limit;
    }
    return res;
}


namespace Test { namespace Obfuscation 
{
    
    using AnyFn = void(*)();       

    struct Entry {
        const char* name;
        AnyFn       fn;    
    };

    static Entry table[] = {
        { "1", reinterpret_cast<AnyFn>(foo1) },
        { "2", reinterpret_cast<AnyFn>(foo2) },
        { "3", reinterpret_cast<AnyFn>(foo3) },
        { "4", reinterpret_cast<AnyFn>(foo4) },
    };



    void Run() 
    {
        int debugCounter = 0; 
        printf("%d\n", debugCounter++);

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

        fprintf(f, "%-6s  %-16s  %-6s  first bytes\n", "func", "address", "len"); 
        fprintf(f, "------  ----------------  ------  ------------------------\n");

        CPolymorphic poly;
        printf("%d\n", debugCounter++);

        const int count = sizeof(table) / sizeof(table[0]);

        for (int i = 0; i < count; ++i) 
        {

            void* addr = (void*)(uintptr_t)table[i].fn;
            uint8_t* p = (uint8_t*)addr;

            // relative JMP (0xE9)
            if (p[0] == 0xE9) 
            {
                int32_t rel = *(int32_t*)(p + 1); // Relative jumps are SIGNED 32-bit
                addr = (void*)((uintptr_t)p + 5 + rel); 
                p = (uint8_t*)addr;
            }

            fprintf(f, "%-6s  %p  ", table[i].name, addr);
            for (int b = 0; b < 8; ++b)
            {
                fprintf(f, "%02X ", p[b]);
            }
            fprintf(f, "\n");

            size_t   len    = Hashing::func_len(addr);
            uint64_t before = Hashing::hash_func(addr);

            printf("mutate%d\n", debugCounter++); 

            // mutate
            DWORD old;
            VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &old);
            poly.Run((uintptr_t)addr);
            VirtualProtect(addr, len, old, &old);

            uint64_t after = Hashing::hash_func(addr);

            fprintf(f, "       len=%-4zu  before=%016llx  after=%016llx  %s\n\n",
                len,
                (unsigned long long)before,
                (unsigned long long)after,
                before == after ? "SAME" : "DIFF");
        }

        fclose(f);
    }
    
}} // namespace Test::Obfuscation

