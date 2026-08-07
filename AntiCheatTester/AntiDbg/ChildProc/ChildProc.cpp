#include "../AntiDbg.h"
#include <iostream>
#include <Windows.h>
#include <sstream>
#include <string>
#include "Vars.h"


HANDLE g_hChild = NULL;
ChildState g_hChildState = UNKNOWN;


void AntiDbg::ChildProc::CreateChildProc(char* argv[])
{
    printf("create child proc..\n");

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    std::stringstream stream;
    stream << GetCurrentProcessId();

    printf("curr proc: %d\n", GetCurrentProcessId());
    
    // setup command args to child proc
    std::string cmdArgs = "\"";
    cmdArgs += argv[0];
    cmdArgs += "\" 1 " + stream.str();

    char* args = new char[cmdArgs.length() + 1];
    strcpy_s(args, cmdArgs.length() + 1, cmdArgs.c_str());

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcessA(NULL, args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        printf( "created child proc\n" );
        g_hChild = pi.hProcess;

        
    }
    else
    {
        printf("%d\n", GetLastError());
    }



    CloseHandle(pi.hThread);
}


void AntiDbg::ChildProc::ChildGuard(int argc, char* argv[])
{
    if (argc < 3) { printf("guard: missing pid arg\n"); exit(1); }

    DWORD pid = atoi(argv[2]);
    if (pid == 0) { printf("guard: bad pid\n"); exit(1); }

    if (DebugActiveProcess(pid))
    {
        printf("[child] ATTACHED to %lu\n", pid);
        DEBUG_EVENT dbgEvent;
        while (WaitForDebugEvent(&dbgEvent, INFINITE))
        {
            DWORD status = (dbgEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
                ? DBG_EXCEPTION_NOT_HANDLED : DBG_CONTINUE;
            ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, status);
        }
    }
    else
    {
        printf("[child] attach FAILED: %lu\n", GetLastError());
    }

}

void AntiDbg::ChildProc::EnsureDebuggingOccurs()
{
    if (g_hChildState != ALIVE) return;   // child confirmed attached first

    BOOL beingDebugged = FALSE;
    if (!CheckRemoteDebuggerPresent(GetCurrentProcess(), &beingDebugged))
    {
        return;
    }

    if (!beingDebugged)
    {
        exit(1);
    }
    printf("[parent] being debugged = %d\n", beingDebugged);
}


void AntiDbg::ChildProc::ValidateAliveChild()
{
    if (g_hChild == NULL)
    {
        return;
    }
    DWORD w = WaitForSingleObject(g_hChild, 0); // poll
    if (w == WAIT_OBJECT_0)
    {
        g_hChildState = DEAD;
    }
    else if (w == WAIT_TIMEOUT)
    {
        g_hChildState = ALIVE;
    }
    else
    {
        g_hChildState = ERR;
    }

}