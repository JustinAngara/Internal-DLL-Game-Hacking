#include <stdio.h>
#include <windows.h>
#include "AntiDbg.h"
#include "../Vars.h"


static NtQueryInformationThread_t fnNtQueryInformationThread = NULL;
static NtSetInformationThread_t   fnNtSetInformationThread   = NULL;

int AntiDbg::RunHideThreadDebugger(LPTHREAD_START_ROUTINE ThreadMain)
{
    DWORD dwThreadId = 0;
    HANDLE hThread = CreateThread(NULL, 0, ThreadMain, NULL, 0, &dwThreadId);
    if (!hThread) return -1;

    HMODULE hDLL = LoadLibraryW(L"ntdll.dll");
    if (!hDLL) return -1;

    fnNtQueryInformationThread = (NtQueryInformationThread_t)GetProcAddress(hDLL, "NtQueryInformationThread");
    fnNtSetInformationThread   = (NtSetInformationThread_t)GetProcAddress(hDLL, "NtSetInformationThread");
    if (!fnNtQueryInformationThread || !fnNtSetInformationThread)
        return -1;

    BOOLEAN bHidden = FALSE;
    ULONG   lRet = 0;

    fnNtSetInformationThread(hThread, ThreadHideFromDebugger, NULL, 0);
    fnNtQueryInformationThread(hThread, ThreadHideFromDebugger, &bHidden, sizeof(bHidden), &lRet);

    printf("Thread is hidden: %s\n", bHidden ? "Yes" : "No");

    WaitForSingleObject(hThread, INFINITE); 
    return 0;
}