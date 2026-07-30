#include <iostream>
#include <Windows.h>
#include "../AntiCheatTester/AntiDbg/AntiDbg.h"
#include "Vars.h"
// anti cheat, anti dbg, obfuscation tester, random stuff3


DWORD WINAPI ThreadMain(LPVOID p) {
    while (true) {
        if (IsDebuggerPresent())
        {
            std::cout << "DEBUGGER PRESENT TEST 1\n";
        }
        Sleep(500);
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