#pragma once

namespace Hashing
{

    inline uint64_t fnv1a(const uint8_t* data, size_t len);

    inline size_t func_len(const void* fn, size_t cap = 512);

    inline uint64_t hash_func(const void* fn);

}
