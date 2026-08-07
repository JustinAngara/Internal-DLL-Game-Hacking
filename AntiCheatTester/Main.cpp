#include <iostream>
#include <Windows.h>
#include <sstream>
#include <string>
#include "../AntiCheatTester/AntiDbg/AntiDbg.h"
#include "Vars.h"
#include "Game/Game.h"
#include "ProcessList/ProcessList.h"

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
    while (true) 
    {
        // just the debuggerpresentmethod
        if (IsDebuggerPresent())
        {
            //std::cout << "DEBUGGER PRESENT TEST";
        }
        else
        {
            //std::cout << "DEBUGGER NOT PRESENT";

        }

        // we give the validation whatever to this
        AntiDbg::ChildProc::ValidateAliveChild();
        AntiDbg::ChildProc::EnsureDebuggingOccurs();
        Sleep(MAX_TIME_TO_SLEEP);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    const char* mode = (argc >= 2) ? argv[1] : "0";

    if (strcmp(mode, "0") == 0)
    {
        AntiDbg::ChildProc::CreateChildProc(argv);
    }
    else
    {
        // this is child proc scope ; dont do anything
        AntiDbg::ChildProc::ChildGuard(argc, argv);   // pass argc through so it can guard too
        return 0; 
    }

    std::cout << "still in working\n";

    // setup threads
    HANDLE hGame    = AntiDbg::RunThreadEx(GameThread);
    HANDLE hAntiDbg = AntiDbg::RunHideThreadDebugger(ThreadMain);

    // execution for threads
    HANDLE handles[] = { hGame, hAntiDbg };
    WaitForMultipleObjects(2, handles, TRUE, INFINITE); 

    CloseHandle(hGame);
    CloseHandle(hAntiDbg);

    ProcList::ListOutProcs();
    
    // ending stub
    std::cout << "Press Enter to Exit.\n";
    std::cin.get();
    
    return 0;
    
}