#include <cstdint>
#include <cstddef>
#include <cstdio>

#include "Obfuscation/Polymorphism/Polymorphic.h"
#include "sdk/Logger/Logger.h"

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
    __asm { mov esi, esi };   // 8B F6 — replaced ebp with esi
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


void EnumerateExports(HMODULE hModule) {
    // DOS header → PE header
    BYTE* base = reinterpret_cast<BYTE*>(hModule);
    IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    IMAGE_NT_HEADERS* nt  = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);

    // locate the export directory
    DWORD exportRVA = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!exportRVA) return; // no exports

    IMAGE_EXPORT_DIRECTORY* expDir =
        reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(base + exportRVA);

    DWORD*  funcRVAs  = reinterpret_cast<DWORD*> (base + expDir->AddressOfFunctions);
    DWORD*  nameRVAs  = reinterpret_cast<DWORD*> (base + expDir->AddressOfNames);
    WORD*   ordinals  = reinterpret_cast<WORD*>  (base + expDir->AddressOfNameOrdinals);

    for (DWORD i = 0; i < expDir->NumberOfNames; ++i) {
        const char* name = reinterpret_cast<const char*>(base + nameRVAs[i]);
        void*       addr = base + funcRVAs[ordinals[i]];

        // follow thunk if present
        uint8_t* p = reinterpret_cast<uint8_t*>(addr);
        if (p[0] == 0xE9) {
            DWORD rel = *reinterpret_cast<DWORD*>(p + 1);
            addr = p + 5 + rel;
        }

        printf("%-40s @ %p\n", name, addr);
    }
}


void RunOnDLL(const char* dllName) {
    HMODULE hMod = GetModuleHandleA(dllName); // already loaded
    // or: LoadLibraryA(dllName) for a DLL on disk

    BYTE* base = reinterpret_cast<BYTE*>(hMod);
    IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    IMAGE_NT_HEADERS* nt  = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);

    DWORD exportRVA = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!exportRVA) return;

    IMAGE_EXPORT_DIRECTORY* expDir =
        reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(base + exportRVA);

    DWORD* funcRVAs = reinterpret_cast<DWORD*>(base + expDir->AddressOfFunctions);
    DWORD* nameRVAs = reinterpret_cast<DWORD*>(base + expDir->AddressOfNames);
    WORD*  ordinals = reinterpret_cast<WORD*> (base + expDir->AddressOfNameOrdinals);

    FILE* f = fopen("recon.txt", "w");
    if (!f) return;

    fprintf(f, "%-40s  %-10s  %-6s  %s\n", "name", "address", "len", "sig");
    fprintf(f, "----------------------------------------  ----------  ------  ----------------\n");

    for (DWORD i = 0; i < expDir->NumberOfNames; ++i) {
        const char* name = reinterpret_cast<const char*>(base + nameRVAs[i]);
        void*       addr = base + funcRVAs[ordinals[i]];

        uint8_t* p = reinterpret_cast<uint8_t*>(addr);
        if (p[0] == 0xE9) {
            DWORD rel = *reinterpret_cast<DWORD*>(p + 1);
            addr = p + 5 + rel;
        }

        size_t   len = func_len(addr);
        uint64_t sig = hash_func(addr);

        fprintf(f, "%-40s  %p  %-6zu  %016llx\n",
            name, addr, len, (unsigned long long)sig);
    }

    fclose(f);
}

namespace Test::Obfuscation { 
    
    struct Entry {
        const char* name;
        void*       fn;

    };

    // table lives here at namespace scope — visible to Run()
    static Entry table[] = {
        { "foo_A", reinterpret_cast<void*>(&foo_A) },
        { "bar_A", reinterpret_cast<void*>(&bar_A) },
        { "foo_B", reinterpret_cast<void*>(&foo_B) },
        { "bar_B", reinterpret_cast<void*>(&bar_B) },
        { "logger", reinterpret_cast<void*>(&Logger::Log) },
        { "logger", reinterpret_cast<void*>(&Logger::Log) },
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

        char outPath[MAX_PATH];
        strcpy_s(outPath, dllPath);
        strcat_s(outPath, MAX_PATH, "recon.txt");

        FILE* f = fopen(outPath, "w");
        if (!f) return;

        BYTE* base = reinterpret_cast<BYTE*>(hSelf);
        IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        IMAGE_NT_HEADERS* nt  = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);


        DWORD exportRVA = nt->OptionalHeader
            .DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

        IMAGE_EXPORT_DIRECTORY* expDir =
            reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(base + exportRVA);


        if (!dos || dos->e_magic != IMAGE_DOS_SIGNATURE) { fclose(f); return; }
        if (!nt || nt->Signature != IMAGE_NT_SIGNATURE) { fclose(f); return; }
        if (!exportRVA) { fprintf(f, "no exports\n"); fclose(f); return; }
        if (!expDir) { fclose(f); return; }


        DWORD* funcRVAs = reinterpret_cast<DWORD*>(base + expDir->AddressOfFunctions);
        DWORD* nameRVAs = reinterpret_cast<DWORD*>(base + expDir->AddressOfNames);
        WORD*  ordinals = reinterpret_cast<WORD*> (base + expDir->AddressOfNameOrdinals);




        fprintf(f, "module  : %s\n",    (char*)(base + expDir->Name));
        fprintf(f, "exports : %lu\n\n", expDir->NumberOfNames);
        fprintf(f, "%-40s  %-10s  %-6s  %-18s  %-18s  %s\n",
            "name", "address", "len", "sig (before)", "sig (after)", "match");
        fprintf(f, "----------------------------------------"
            "  ----------  ------  ------------------"
            "  ------------------  -----\n");

        CPolymorphic poly;

        for (DWORD i = 0; i < expDir->NumberOfNames; ++i) {
            const char* name = reinterpret_cast<const char*>(base + nameRVAs[i]);
            void*       addr = base + funcRVAs[ordinals[i]];

            // skip forwarded exports
            DWORD rva     = funcRVAs[ordinals[i]];
            DWORD expSize = nt->OptionalHeader
                .DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
            if (rva >= exportRVA && rva < exportRVA + expSize) continue;

            // follow thunk
            uint8_t* p = reinterpret_cast<uint8_t*>(addr);
            if (p[0] == 0xE9) {
                DWORD rel = *reinterpret_cast<DWORD*>(p + 1);
                addr = p + 5 + rel;
            }

            size_t   len    = func_len(addr);
            uint64_t before = hash_func(addr);

            DWORD old;
            VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &old);
            poly.Run((DWORD)addr);
            VirtualProtect(addr, len, old, &old);

            uint64_t after = hash_func(addr);

            fprintf(f, "%-40s  %p  %-6zu  %016llx  %016llx  %s\n",
                name, addr, len,
                (unsigned long long)before,
                (unsigned long long)after,
                before == after ? "SAME" : "DIFF");
        }

        fclose(f);
    }

    
} 