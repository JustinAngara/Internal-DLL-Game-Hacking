#include "Griefer.h"


static volatile unsigned int g_floodSink;

int Griefer::FloodCallStack(volatile int x)
{
    volatile unsigned char waste[8192];

    unsigned int acc = static_cast<unsigned int>(x);
    acc ^= (acc << 13);
    acc ^= (acc >> 17);
    acc ^= (acc << 5);

    waste[acc % sizeof(waste)] = static_cast<unsigned char>(acc);
    waste[static_cast<unsigned int>(x) % sizeof(waste)] = static_cast<unsigned char>(x);
    g_floodSink = acc + waste[0];

    if (acc & 1u)
        return FloodCallStack(x + 1) + FloodCallStack(static_cast<int>(acc ^ 0xABCDu)) + waste[1];
    else
        return FloodCallStack(x + 1) + waste[2];
}
