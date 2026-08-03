#include <iostream>
#include <Windows.h>
#include "../AntiCheatTester/AntiDbg/AntiDbg.h"
#include "Vars.h"
#include "Game/Game.h"
// anti cheat, anti dbg, obfuscation tester, random stuff3

constexpr int MAX_TIME_TO_SLEEP = 5000;

Game* Game::s_instance = nullptr;
static Game g_game;          

DWORD WINAPI GameThread(LPVOID p)
{
    Game::s_instance = &g_game;

    Func fGameIterator = &Game::RunTrampoline;   

    while (true)
    {
        std::cout << "\n\n\n";
        if (AntiDbg::CheckForDebugger(fGameIterator, AntiDbg::TICK_COUNT) > 5000)
        {
            std::cout << "\nDEBUGGER:\n->TIME FLAGGED\n";
        } 
        else
        {
            std::cout << "\nDEBUGGER:\n->TIME NOT FLAGGED\n";
        }
        
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
            //std::cout << "DEBUGGER PRESENT TEST";
        }
        else
        {
            //std::cout << "DEBUGGER NOT PRESENT";

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