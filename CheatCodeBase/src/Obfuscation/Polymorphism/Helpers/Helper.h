#include "../Polymorphic.h"

namespace Helper
{

    namespace Hashing
    {
        inline uint64_t fnv1a(const uint8_t* data, size_t len);

        inline size_t func_len(const void* fn, size_t cap = 512);

        inline uint64_t hash_func(const void* fn);

    }

    namespace SEH
    {
        bool SafeReadBytes(const uint8_t* p, uint8_t* out, int n);
        bool SafeMeasure(void* addr, size_t* pLen, uint64_t* pHash);
        bool SafeHash(void* addr, uint64_t* pHash);

        // poly is passed by pointer; the call is the only thing guarded.
        bool SafeMutate(CPolymorphic* poly, uintptr_t addr);

        void __try_reloc(CPolymorphic* poly, uintptr_t addr, size_t len, uintptr_t* out);

    }

}