namespace Memory
{
	

    inline unsigned char Read (uintptr_t dwAddress)
    {
        return *(unsigned char*)dwAddress;
    }

    inline void Write (uintptr_t dwAddress, unsigned char val)
    {
        *(unsigned char*)dwAddress = val;
    }

    inline int randomize(int min, int max)
    {
        return (1 + int((max - min + 1)*rand()/(RAND_MAX + 1.0)));
    }

    inline int oplen(BYTE* address) 
    {
#if defined(_M_IX86)
        hde32s hs;
        unsigned int len = hde32_disasm((void*)address, &hs);
        if (hs.flags & F_ERROR) return 0;
        return len;

#elif defined(_M_X64) || defined(_M_AMD64)
        hde64s hs;
        unsigned int len = hde64_disasm((void*)address, &hs);
        if (hs.flags & F_ERROR) return 0;
        return len;

#else
        return 0; 
#endif
    }





    void Mutate1Byte(uintptr_t dwAddress, uint8_t b1);

    void Mutate2Byte(uintptr_t dwAddress, uint8_t b1);

    void Mutate3Byte(uintptr_t dwAddress, uint8_t b1);

    void Mutate5Byte(uintptr_t dwAddress, uint8_t b1);

    void Mutate6Byte(uintptr_t dwAddress, uint8_t b1);

    void Mutate80(uintptr_t dwAddress, uint8_t b1);

    void Mutate8BitRegToReg(uintptr_t dwAddress, uint8_t b1);

    void Mutate8BitAccImm(uintptr_t dwAddress, uint8_t b1);

    void MutateTwoByte(uintptr_t dwAddress, bool pfx66, bool pfxF2, bool pfxF3);

}