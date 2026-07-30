#include <iostream>
#include <Windows.h>
#include "../AntiCheatTester/AntiDbg/AntiDbg.h"
#include "Vars.h"
#include "Game/Game.h"
// anti cheat, anti dbg, obfuscation tester, random stuff3

constexpr int MAX_TIME_TO_SLEEP = 5000;


DWORD WINAPI ThreadMain(LPVOID p) 
{

    Game g;
    Game::s_instance = &g;

    Func fGameIterator = &Game::RunTrampoline;   



    // pseudo game loop
    while (true) {
        // just the debuggerpresentmethod
        if (IsDebuggerPresent())
        {
            std::cout << "DEBUGGER PRESENT TEST";
        }

        // We are going to use our debugger tester and pass in our game loop to see 
        // if a debugger is present in respect to time
        if (AntiDbg::CheckForDebugger(fGameIterator, AntiDbg::TICK_COUNT) > MAX_TIME_TO_SLEEP)
        {
            std::cout << "DEBUGGERR PRESENT\n";
        }

        printf("\nPress enter to move to next iteration.\n");

        std::cin.get();

    }
    return 0;
}

int main()
{
	std::cout << "still in working\n";

    // this will be able to hook isdebuggerpresent 
	 AntiDbg::RunHideThreadDebugger(ThreadMain);



	printf("Press enter to exit.\n");
	
	return 0;
}