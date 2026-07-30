#include <cstdint>
#include <cstdio>

#include "Obfuscation/Polymorphism/Polymorphic.h"
#include "Obfuscation/Polymorphism/Helpers/Helper.h"
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
        char dllPath[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            (LPCSTR)&Run, &hSelf);
        GetModuleFileNameA(hSelf, dllPath, MAX_PATH);
        char* slash = strrchr(dllPath, '\\');
        if (slash) *(slash + 1) = '\0';
        strcat_s(dllPath, MAX_PATH, "signatures.txt");

        FILE* f = fopen(dllPath, "w");
        if (!f) {
            printf("fopen FAILED: %s (errno=%d)\n", dllPath, errno);
            return;
        }
        printf("fopen OK: %s\n", dllPath);

        fprintf(f, "%-6s  %-18s  %-6s  first bytes\n", "func", "address", "len");
        fprintf(f, "------  ------------------  ------  ------------------------\n");
        fflush(f);

        CPolymorphic poly;
        const int count = sizeof(table) / sizeof(table[0]);

        for (int i = 0; i < count; ++i)
        {
            void*    addr = (void*)(uintptr_t)table[i].fn;
            uint8_t* p    = (uint8_t*)addr;

            // Resolve incremental-link thunks (chase chains, don't stop at first).
            int guard = 0;
            while (p[0] == 0xE9 && guard++ < 8) 
            {
                int32_t rel = *(int32_t*)(p + 1);
                addr = (void*)((uintptr_t)p + 5 + rel);
                p    = (uint8_t*)addr;
            }

            // First 8 bytes (guarded via helper; no C++ objects in the __try).
            uint8_t bytes[8] = {0};
            bool readOk = Helper::SEH::SafeReadBytes(p, bytes, 8);

            fprintf(f, "%-6s  %p  ", table[i].name, addr);
            if (readOk)
                for (int b = 0; b < 8; ++b) fprintf(f, "%02X ", bytes[b]);
            else
                fprintf(f, "<read fault>");
            fprintf(f, "\n");
            fflush(f);

            // Length + before-hash.
            size_t   len    = 0;
            uint64_t before = 0;
            if (!Helper::SEH::SafeMeasure(addr, &len, &before)) 
            {
                fprintf(f, "       <func_len/hash faulted>\n\n");
                fflush(f);
                continue;
            }

            if (len == 0 || len > 0x4000) 
            {
                fprintf(f, "       len=%-4zu  SKIPPED (bad length)\n\n", len);
                fflush(f);
                continue;
            }

            // --- Make the code writable, from the PAGE BASE, padded so the
            //     handlers' addr-1 writes and forward imm reads stay in range. ---
            uintptr_t pageBase = (uintptr_t)addr & ~(uintptr_t)0xFFF;
            SIZE_T    span     = (uintptr_t)addr + len - pageBase + 0x10;

            DWORD prev = 0, tmp = 0;
            
            if (!VirtualProtect((void*)pageBase, span, PAGE_EXECUTE_READWRITE, &prev)) 
            {
                fprintf(f, "       len=%-4zu  VirtualProtect FAILED (err=%lu)\n\n",
                    len, GetLastError());
                fflush(f);
                continue;
            }

            bool mutateOk = Helper::SEH::SafeMutate(&poly, (uintptr_t)addr);

            VirtualProtect((void*)pageBase, span, prev, &tmp);
            FlushInstructionCache(GetCurrentProcess(), addr, len);

            uint64_t after = before;
            Helper::SEH::SafeHash(addr, &after);

            // If in-place left it unchanged, try relocate-with-junk (safe copy).
            const char* method = "inplace";
            if (mutateOk && before == after) 
            {
                uintptr_t relocated = 0;
                Helper::SEH::__try_reloc(&poly, (uintptr_t)addr, len, &relocated);  // SEH helper
                if (relocated) 
                {
                    table[i].fn = reinterpret_cast<AnyFn>(relocated);  // repoint
                    after  = Helper::Hashing::hash_func((void*)relocated);     // hash the copy
                    addr   = (void*)relocated;
                    method = "reloc+junk";
                }
            }

            const char* status = !mutateOk ? "CRASH"
                : (before == after) ? "SAME" : "DIFF";

            fprintf(f, "       len=%-4zu  before=%016llx  after=%016llx  %-4s  [%s]\n\n",
                len, (unsigned long long)before, (unsigned long long)after,
                status, method);
            fflush(f);
        }

        fclose(f);
        printf("done\n");
    }
    
}} // namespace Test::Obfuscation




namespace Test{namespace Utils
{
    bool IsInValidMemoryRegion(uint64_t return_address)
    {
        uint64_t base = (uint64_t)GetModuleHandleA(nullptr);
        PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
        PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
        return (return_address > base && return_address < (base + nt->OptionalHeader.SizeOfImage));
    }
}}