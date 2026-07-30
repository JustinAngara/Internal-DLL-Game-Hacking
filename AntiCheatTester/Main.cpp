#include <iostream>
#include <Windows.h>
#include "../AntiCheatTester/AntiDbg/AntiDbg.h"
#include "Vars.h"
// anti cheat, anti dbg, obfuscation tester, random stuff3


DWORD WINAPI ThreadMain(LPVOID p) {
    // game logic, alongside the game  
    while (true) {
        // just the debuggerpresentmethod
        if (IsDebuggerPresent())
        {
            std::cout << "DEBUGGER PRESENT TEST 1\n";
        }
        

        // using tick count alongside of the game iteration
        // maybe we can do IsDebuggerPresent(gameIterativeFuncPointer..., IsDebuggerPResent enumMethod), then run arbritrary pauses

    }
    return 0;
}

int main()
{
	std::cout << "still in working\n";

	AntiDbg::RunHideThreadDebugger(ThreadMain);



	printf("Press enter to exit.\n");
	std::cin.get();

	return 0;
}