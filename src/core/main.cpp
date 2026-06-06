#include "sdk/Logger/Logger.h"
#include "Main.h"
#include "sdk/Memory/PatternScan.h"
#include <Windows.h>
#include <iostream>
void Main::entrypoint()
{

    // create console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);


	assaultCubeHack(); // will run assault cube

    // close console
    fclose(f);
    FreeConsole();
}


void Main::assaultCubeHack()
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

            if (bRecoil)
            {
                //mem::Nop((BYTE*)(moduleBase + 0x63786), 10);
            }
            else
            {
                //50 8D 4C 24 1C 51 8B CE FF D2 the original stack setup and call
                //PatternScan::Patch((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2", 10);
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

