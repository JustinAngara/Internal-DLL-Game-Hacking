#include <iostream>
#include <Windows.h>
#include "../AntiCheatTester/AntiDbg/AntiDbg.h"
#include "Vars.h"
#include "Game/Game.h"
// anti cheat, anti dbg, obfuscation tester, random stuff3


DWORD WINAPI ThreadMain(LPVOID p) {

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
        if (AntiDbg::CheckForDebugger(fGameIterator, AntiDbg::TICK_COUNT) > 5000)
        {
            std::cout << "DEBUGGERR PRESENT\n";
        }





        Sleep(3000);// more game loop to check out cheat engine and stuff

    }
    return 0;
}

int main()
{
	std::cout << "still in working\n";

    // this will be able to hook isdebuggerpresent 
	AntiDbg::RunHideThreadDebugger(ThreadMain);



	printf("Press enter to exit.\n");
	std::cin.get();

	return 0;
}