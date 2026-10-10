#pragma once

namespace Hashing
{
    // focus on developing later
    inline uint64_t fnv1a(const uint8_t* data, size_t len);

    inline size_t func_len(const void* fn, size_t cap = 512);

    inline uint64_t hash_func(const void* fn);

    // arbritrary patch
    // click run/next
    // check if they compare and its bytes are restored back to where they are
}
