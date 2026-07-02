//#include "Polymorphic.h"
//
//#include <iostream>
//
//void CPolymorphic::ObfuscateOpcode(DWORD dwAddress, int opcodeLen)
//{
//    if(opcodeLen == 1)
//    {
//        if ((this->randomize(1, 2) == 1) && (this->Read(dwAddress) == 0xC3 && this->Read(dwAddress + 1) == 0xCC && this->Read(dwAddress + 2) == 0xCC)) // retn -> retn 0
//        {
//            std::cout << std::hex << "retn -> retn 0" << dwAddress << '\n';
//
//            this->Write(dwAddress, 0xC2);
//            this->Write(dwAddress + 1, 0x00);
//            this->Write(dwAddress + 2, 0x00);
//        }
//        else if (this->Read(dwAddress) == 0xCC) // int 3
//        {
//            if (this->Read(dwAddress + 1) == 0xCC)
//            {
//                if (this->randomize(1, 10) > 5)
//                {
//                    std::cout << std::hex << "0xCC -> mov reg, reg" << dwAddress << '\n';
//
//                    this->Write(dwAddress, 0x8B); // mov
//                    this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//                }
//                else
//                {
//                    std::cout << std::hex << "0xCC -> rand at 0x" << dwAddress << '\n';
//
//                    this->Write(dwAddress, this->randomize(0, 0xFF));
//                }
//            }
//            else
//            {
//                std::cout << std::hex << "0xCC -> rand at 0x" << dwAddress << '\n';
//
//                this->Write(dwAddress, this->randomize(0, 0xFF));
//            }
//        }
//        else if (this->Read(dwAddress) == 0x90 && this->Read(dwAddress + 1) == 0x90) // nop
//        {
//            std::cout << std::hex << "0x90 0x90 -> mov reg, reg at 0x" << dwAddress << '\n';
//
//            this->Write(dwAddress, 0x8B); // mov
//            this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//        }
//    }
//    else if (opcodeLen == 2)
//    {
//        if (this->Read(dwAddress) == 0x8B) // mov
//        {
//            if (this->Read(dwAddress + 1) == 0xC0) // mov eax, eax
//            {
//                std::cout << "patched mov eax, eax" << std::endl;
//
//                if (this->randomize(1, 10) > 4)
//                {
//                    this->Write(dwAddress, 0x8B); // mov
//                    this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//                }
//                else
//                {
//                    this->Write(dwAddress, 0x90); // nop
//                    this->Write(dwAddress + 1, 0x90); // nop
//                }
//            }
//            else if (this->Read(dwAddress + 1) == 0xDB) // mov ebx, ebx
//            {
//                std::cout << "patched mov ebx, ebx" << std::endl;
//
//                if (this->randomize(1, 10) > 4)
//                {
//                    this->Write(dwAddress, 0x8B); // mov
//                    this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//                }
//                else
//                {
//                    this->Write(dwAddress, 0x90); // nop
//                    this->Write(dwAddress + 1, 0x90); // nop
//                }
//            }
//            else if (this->Read(dwAddress + 1) == 0xC9) // mov ecx, ecx
//            {
//                std::cout << "patched mov ecx, ecx" << std::endl;
//
//                if (this->randomize(1, 10) > 4)
//                {
//                    this->Write(dwAddress, 0x8B); // mov
//                    this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//                }
//                else
//                {
//                    this->Write(dwAddress, 0x90); // nop
//                    this->Write(dwAddress + 1, 0x90); // nop
//                }
//            }
//            else if (this->Read(dwAddress + 1) == 0xD2) // mov edx, edx
//            {
//                std::cout << "patched mov edx, edx" << std::endl;
//
//                if (this->randomize(1, 10) > 4)
//                {
//                    this->Write(dwAddress, 0x8B); // mov
//                    this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//                }
//                else
//                {
//                    this->Write(dwAddress, 0x90); // nop
//                    this->Write(dwAddress + 1, 0x90); // nop
//                }
//            }
//            else if (this->Read(dwAddress + 1) == 0xE4) // mov esp, esp
//            {
//                std::cout << "patched mov esp, esp" << std::endl;
//
//                if (this->randomize(1, 10) > 4)
//                {
//                    this->Write(dwAddress, 0x8B); // mov
//                    this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//                }
//                else
//                {
//                    this->Write(dwAddress, 0x90); // nop
//                    this->Write(dwAddress + 1, 0x90); // nop
//                }
//            }
//            else if (this->Read(dwAddress + 1) == 0xED) // mov ebp, ebp
//            {
//                std::cout << "patched mov ebp, ebp" << std::endl;
//
//                if (this->randomize(1, 10) > 4)
//                {
//                    this->Write(dwAddress, 0x8B); // mov
//                    this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//                }
//                else
//                {
//                    this->Write(dwAddress, 0x90); // nop
//                    this->Write(dwAddress + 1, 0x90); // nop
//                }
//            }
//            else if (this->Read(dwAddress + 1) == 0xF6) // mov esi, esi
//            {
//                std::cout << "patched mov esi, esi" << std::endl;
//
//                if (this->randomize(1, 10) > 4)
//                {
//                    this->Write(dwAddress, 0x8B); // mov
//                    this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//                }
//                else
//                {
//                    this->Write(dwAddress, 0x90); // nop
//                    this->Write(dwAddress + 1, 0x90); // nop
//                }
//            }
//            else if (this->Read(dwAddress + 1) == 0xFF) // mov edi, edi
//            {
//                std::cout << "patched mov edi, edi" << std::endl;
//
//                if (this->randomize(1, 10) > 4)
//                {
//                    this->Write(dwAddress, 0x8B); // mov
//                    this->Write(dwAddress + 1, regs32[this->randomize(0, 7)]); // reg
//                }
//                else
//                {
//                    this->Write(dwAddress, 0x90); // nop
//                    this->Write(dwAddress + 1, 0x90); // nop
//                }
//            }
//        }
//
//
//    }
//
//}
//
//void CPolymorphic::Run(DWORD dwStart)
//{
//    DWORD dwLength = CalculateFunctionSize(dwStart);
//    DWORD dwCurrent = dwStart;
//    MEMORY_BASIC_INFORMATION mbi;
//
//    while (true)
//    {
//        VirtualQuery((void*)dwCurrent, &mbi, sizeof(mbi));
//
//        // Check if the memory page is mapped and accessible
//        if (mbi.State == MEM_COMMIT && mbi.Protect != 0x1)
//        {
//            DWORD prevAccess, newAccess, startAddress(dwCurrent);
//
//            // Enable writing access to the code
//            VirtualProtect((PBYTE)dwCurrent, ((DWORD)mbi.BaseAddress + mbi.RegionSize) - startAddress, PAGE_EXECUTE_READWRITE, &prevAccess);
//
//            while ((dwCurrent < (DWORD)mbi.BaseAddress + static_cast<DWORD>(mbi.RegionSize)) && (dwCurrent < dwStart + dwLength))
//            {
//                int nOpcodeLen = oplen((BYTE*)dwCurrent);
//
//                if (nOpcodeLen > 0)
//                {
//                    this->ObfuscateOpcode(dwCurrent, nOpcodeLen);
//                    dwCurrent += nOpcodeLen;
//                }
//                else
//                    dwCurrent++;
//            }
//
//            // Apply old access to the code
//            VirtualProtect((PBYTE)startAddress, ((DWORD)mbi.BaseAddress + mbi.RegionSize) - startAddress, prevAccess, &newAccess);
//        }
//        else
//            dwCurrent = (DWORD)mbi.BaseAddress + mbi.RegionSize + 0x1; // Go to the next region
//
//        if (dwCurrent >= dwStart + dwLength)
//            break;
//    }
//}
//
//DWORD CPolymorphic::CalculateFunctionSize(DWORD_PTR dwStart)
//{
//    DWORD dwMaxBytes = 4096;
//    DWORD_PTR dwCurrent = dwStart;
//    DWORD dwLength = 0;
//    DWORD dwLastRetOffset = 0;
//    bool bFoundRet = false;
//
//    while (dwLength < dwMaxBytes) {
//        BYTE currentByte = *(BYTE*)dwCurrent;
//
//        bool isRet = (currentByte == 0xC3 ||  // near RET
//            currentByte == 0xC2 ||  // near RET imm16
//            currentByte == 0xCB ||  // far RETF
//            currentByte == 0xCA);   // far RETF imm16
//
//        int instrLen = oplen((BYTE*)dwCurrent);
//
//        if (instrLen == 0) break; // unknown instruction, bail
//
//        dwLength += instrLen;
//        dwCurrent += instrLen;
//
//        if (isRet) {
//            dwLastRetOffset = dwLength;
//            bFoundRet = true;
//
//            // if next byte looks like a new function prologue, stop
//            BYTE nextByte = *(BYTE*)dwCurrent;
//            if (nextByte == 0x55 || nextByte == 0xCC) // PUSH EBP or INT3 padding
//            {
//                break;
//            }
//        }
//    }
//
//    // return length through the last RET we found, not just the first
//    return bFoundRet ? dwLastRetOffset : dwLength;
//}
//CPolymorphic::CPolymorphic(void)
//{
//    srand((unsigned)time(0));
//
//    this->regs32[0] = 0xC0;
//    this->regs32[1] = 0xDB;
//    this->regs32[2] = 0xC9;
//    this->regs32[3] = 0xD2;
//    this->regs32[4] = 0xE4;
//    this->regs32[5] = 0xED;
//    this->regs32[6] = 0xF6;
//    this->regs32[7] = 0xFF;
//
//}
//
//CPolymorphic::~CPolymorphic(void)
//{
//}