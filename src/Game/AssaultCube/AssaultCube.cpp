#include "AssaultCube.h"
#include "sdk/Memory/Memory.h"
void AssaultCube::run() const
{
    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");


    moduleBase = (uintptr_t)GetModuleHandle(NULL);
    bool bHealth = false, bAmmo = false, bRecoil = false;

    while (true)
    {
        // add the keylisteners
        if (GetAsyncKeyState(VK_END) & 1)
        {
            break;
        }

        if (GetAsyncKeyState(VK_NUMPAD1) & 1)
            bHealth = !bHealth;

        if (GetAsyncKeyState(VK_NUMPAD2) & 1)
        {
            bAmmo = !bAmmo;
        }

        //no recoil NOP
        if (GetAsyncKeyState(VK_NUMPAD3) & 1)
        {
            bRecoil = !bRecoil;
            int recoilInstrOffset   = 0x63786;
            int recoilInstrSize     = 10;
            BYTE* recoilBytePattern = (BYTE*)"\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2";
            
            if (bRecoil)
            {
                Memory::Nop((BYTE*)(moduleBase + recoilInstrOffset), recoilInstrSize);
            }
            else
            {
                Memory::Patch((BYTE*)(moduleBase + recoilInstrOffset), recoilBytePattern, recoilInstrSize);
            }
        }

        uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x10F4F4);

        if (localPlayerPtr)
        {
            if (bHealth)
            {

                *(int*)(*localPlayerPtr + 0xF8) = 1337;
            }

            if (bAmmo)
            {
                uintptr_t ammoAddr = PatternScan::FindDMAAddy(moduleBase + 0x10F4F4, { 0x374, 0x14, 0x0 });
                int* ammo = (int*)ammoAddr;
                *ammo = 1337;
            }

        }
        Sleep(5);
    }   

}