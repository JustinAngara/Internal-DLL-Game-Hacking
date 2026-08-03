#include <iostream>
#include <Windows.h>
#include "../AntiCheatTester/AntiDbg/AntiDbg.h"
#include "Vars.h"
#include "Game/Game.h"
// anti cheat, anti dbg, obfuscation tester, random stuff3

constexpr int MAX_TIME_TO_SLEEP = 5000;


DWORD WINAPI GameThread(LPVOID p)
{
    std::cout << "we are in game thread\n";
    Game g;
    Game::s_instance = &g;

    Func fGameIterator = &Game::RunTrampoline;   

    while (true)
    {
        
        fGameIterator();
        
        printf("\nPress enter to move to next iteration.\n");

        std::cin.get();

    }
}
DWORD WINAPI ThreadMain(LPVOID p) 
{
    // this is where the anti cheat would live for repeated calls
    while (true) {
        // just the debuggerpresentmethod
        if (IsDebuggerPresent())
        {
            std::cout << "DEBUGGER PRESENT TEST";
        }
        else
        {
            std::cout << "DEBUGGER NOT PRESENT";

        }
        
        Sleep(5000);
        
    }
    return 0;
}

int main()
{
    std::cout << "still in working\n";

    HANDLE hGame    = AntiDbg::RunThreadEx(GameThread);
    HANDLE hAntiDbg = AntiDbg::RunHideThreadDebugger(ThreadMain);

    HANDLE handles[] = { hGame, hAntiDbg };
    WaitForMultipleObjects(2, handles, TRUE, INFINITE); 

    CloseHandle(hGame);
    CloseHandle(hAntiDbg);
    return 0;
}