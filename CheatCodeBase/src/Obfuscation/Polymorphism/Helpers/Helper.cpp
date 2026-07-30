#include "Helper.h"


inline uint64_t Helper::Hashing::fnv1a(const uint8_t* data, size_t len) 
{
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; ++i) 
    {
        h ^= data[i];
        h *= 1099511628211ULL;
    }
    return h;
}

inline size_t Helper::Hashing::func_len(const void* fn, size_t cap) 
{
    const uint8_t* p = reinterpret_cast<const uint8_t*>(fn);
    for (size_t i = 0; i < cap; ++i)
    {
        if (p[i] == 0xC3) return i + 1;
    }
    return cap;
}

inline uint64_t Helper::Hashing::hash_func(const void* fn) 
{
    const uint8_t* p = reinterpret_cast<const uint8_t*>(fn);
    size_t len = func_len(fn);

    uint64_t h = 0;
    for (size_t i = 0; i < len; ++i)
    {
        h += (uint64_t)p[i] * (i + 1);   
    }

    return h;
}



/////// seh
bool Helper::SEH::SafeReadBytes(const uint8_t* p, uint8_t* out, int n)
{
    __try 
    {
        for (int i = 0; i < n; ++i) out[i] = p[i];
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool Helper::SEH::SafeMeasure(void* addr, size_t* pLen, uint64_t* pHash)
{
    __try
    {
        *pLen = Hashing::func_len(addr);
        *pHash = Hashing::hash_func(addr);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool Helper::SEH::SafeHash(void* addr, uint64_t* pHash)
{
    __try 
    {
        *pHash = Hashing::hash_func(addr);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        return false;
    }
}


// poly is passed by pointer; the call is the only thing guarded.
bool Helper::SEH::SafeMutate(CPolymorphic* poly, uintptr_t addr)
{
    __try 
    {
        poly->Run(addr);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        return false;
    }
}



void Helper::SEH::__try_reloc(CPolymorphic* poly, uintptr_t addr, size_t len, uintptr_t* out)
{
    *out = 0;
    __try 
    { 
        *out = poly->RelocateWithJunk(addr, len); 
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    { 
        *out = 0; 
    }
}