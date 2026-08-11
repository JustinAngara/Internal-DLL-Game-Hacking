#include "../AntiDbg.h"
#include <iostream>
#include <Windows.h>
#include <sstream>
#include <string>
#include <vector>
#include "Vars.h"


static HANDLE     g_hChild      = NULL;
static HANDLE     g_hAttached   = NULL; 
static ChildState g_hChildState = UNKNOWN;

static std::wstring AttachEventName(DWORD pid)
{
    return L"Local\\ChildAttached_" + std::to_wstring(pid);
}


void AntiDbg::ChildProc::CreateChildProc(char* argv[])
{
    printf("create child proc..\n");

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    std::stringstream stream;
    stream << GetCurrentProcessId();

    printf("curr proc: %d\n", GetCurrentProcessId());

    // parent owns the event; child opens it by name after a successful attach
    g_hAttached = CreateEventW(NULL, TRUE, FALSE, AttachEventName(GetCurrentProcessId()).c_str());
    if (g_hAttached == NULL)
    {
        printf("CreateEvent failed: %lu\n", GetLastError());
        return;
    }

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
        printf("created child proc\n");
        g_hChild = pi.hProcess;
        CloseHandle(pi.hThread);
    }
    else
    {
        std::cout << "couldn't call createchildprocA\n";
        printf("%d\n", GetLastError());
    }

    delete[] args;
}

bool AntiDbg::ChildProc::WaitForChildAttach(DWORD timeoutMs)
{
    if (g_hAttached == NULL) return false;

    if (WaitForSingleObject(g_hAttached, timeoutMs) == WAIT_OBJECT_0)
    {
        g_hChildState = ALIVE;
        return true;
    }

    printf("[parent] child never confirmed attach\n");
    return false;
}


void AntiDbg::ChildProc::ChildGuard(int argc, char* argv[])
{
    if (argc < 3) { printf("guard: missing pid arg\n"); exit(1); }

    DWORD pid = (DWORD)atoi(argv[2]);

    std::vector<std::string_view> args(argv, argv + argc);
    for (const auto& arg : args)
        std::cout << arg << "\n";

    if (pid == 0) { printf("guard: bad pid\n"); exit(1); }

    if (DebugActiveProcess(pid))
    {
        printf("[child] ATTACHED to %lu\n", pid);

        // tell the parent we're in
        HANDLE e = OpenEventW(EVENT_MODIFY_STATE, FALSE, AttachEventName(pid).c_str());
        if (e)
        {
            SetEvent(e);
            CloseHandle(e);
        }

        DEBUG_EVENT dbgEvent;
        while (WaitForDebugEvent(&dbgEvent, INFINITE))
        {
            DWORD status = DBG_CONTINUE;
            if (dbgEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
            {
                ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, status);
                break; // parent gone, stop guarding
            }
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
