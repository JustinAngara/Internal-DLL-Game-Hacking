#include <cstdint>
#include <cstdio>

#include "Obfuscation/Polymorphism/Polymorphic.h"
#include "sdk/Math/Math.h"

#include <Windows.h>   
#include <cstring>     

namespace Test { namespace Obfuscation 
{

    using AnyFn = void(*)();       

    struct Entry {
        const char* name;
        AnyFn       fn;    
    };

    static Entry table[] = {
        { "math", reinterpret_cast<AnyFn>(Math::Vec3::Normalize) },

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

            size_t   len    = Hashing::func_len(addr);
            uint64_t before = Hashing::hash_func(addr);

            // mutate
            DWORD old;
            VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &old);
            poly.Run((DWORD)addr);
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

