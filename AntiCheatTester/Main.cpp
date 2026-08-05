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

// headers
void createChildProc(char* argv[]);
void childGuard(char* argv[]);

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


void createChildProc(char* argv[])
{
    printf( "create child proc..\n" );

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    std::stringstream stream;
    stream << GetCurrentProcessId();

    printf( "curr proc: %d\n", GetCurrentProcessId() );
    std::string cmdArgs( argv[0] );
    cmdArgs+= " 1 " + stream.str();
    char* args = new char[cmdArgs.length() + 1];
    strcpy_s( args, cmdArgs.length() + 1, cmdArgs.c_str());

    ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof( si );
    ZeroMemory( &pi, sizeof( pi ) );

    if (CreateProcessA(NULL, args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        printf( "succeded\n" );
    }
    else
    {
        printf( "%d\n", GetLastError() );
    }


    while (GetProcessId( pi.hProcess )) 
    { 
        Sleep( 2000 );
    }

    CloseHandle(pi.hThread);
    exit( -1 );
}


void childGuard(int argc, char* argv[])
{
    if (argc < 3) { printf("guard: missing pid arg\n"); exit(1); }

    DWORD pid = atoi(argv[2]);
    HANDLE handle = OpenProcess( PROCESS_ALL_ACCESS, false, pid );
    if (pid != 0 || handle != INVALID_HANDLE_VALUE) {
        if (!DebugActiveProcess(pid))
        {
            TerminateProcess( handle, -2 );
        }
        DWORD exitCode;

        while (GetExitCodeProcess( handle, &exitCode )) 
        {
            // idle
            Sleep( 2000 );
        }
    }

    exit( -1 );
}

int main(int argc, char* argv[])
{
    const char* mode = (argc >= 2) ? argv[1] : "0";

    if (strcmp(mode, "0") == 0)
    {
        createChildProc(argv);
    }
    else
    {
        childGuard(argc, argv);   // pass argc through so it can guard too
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
    std::cin;
    
    return 0;
    
}