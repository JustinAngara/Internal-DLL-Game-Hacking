#include <Windows.h>
#include "Main.h"
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

DWORD WINAPI CS2::CounterStrikeThread(LPVOID lpParam)
{
    HMODULE hModule = static_cast<HMODULE>(lpParam);

    AllocConsole();
    FILE* console;
    freopen_s(&console, "CONOUT$", "w", stdout);

    std::cout << "hello!\n";

    while (true) 
    {
        if (GetAsyncKeyState(VK_END) & 0x8000) {
            break; 
        }

        Sleep(10); 
    }

    if (console) 
    {
        fclose(console);
    }
    FreeConsole();

    FreeLibraryAndExitThread(hModule, 0);
    return 0; 
}

