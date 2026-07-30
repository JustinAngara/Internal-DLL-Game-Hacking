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


int AntiDbg::CheckForTickCount(Func func)
{

    DWORD tickReference = GetTickCount();
    printf("\nbefore: %d\n", tickReference);

    func(); 

    DWORD currentTick = GetTickCount();
    printf("\nafter: %d\n", currentTick);

    DWORD elapsedTime = currentTick - tickReference;
    printf("\ndifference: %u", elapsedTime);


    return elapsedTime;
}
 

int AntiDbg::CheckForLocalTime(Func func) 
{
    SYSTEMTIME sysStart, sysend;
    FILETIME fStart, fEnd;
    ULARGE_INTEGER uiStart, uiEnd;

    GetLocalTime(&sysStart);

    func();

    GetLocalTime(& sysend);

    if (!SystemTimeToFileTime(&sysend, &fEnd))
        return false;
    if (!SystemTimeToFileTime(&sysStart, &fStart))
        return false;

    uiStart.LowPart = fStart.dwLowDateTime;
    uiStart.HighPart = fStart.dwHighDateTime;
    uiEnd.LowPart = fEnd.dwLowDateTime;
    uiEnd.HighPart = fEnd.dwHighDateTime;

    return (((uiEnd.QuadPart - uiStart.QuadPart)*100)/1000000); // filetime conversion to ms
} 


int AntiDbg::CheckForQPC(Func func) 
{
    LARGE_INTEGER start, end, frequency;
    QueryPerformanceCounter(&start);
    QueryPerformanceFrequency(&frequency);

    func();

    QueryPerformanceCounter(&end);
    return (end.QuadPart - start.QuadPart)*1000/frequency.QuadPart;
}


int AntiDbg::CheckForDebugger(Func func, AntiDbgMethod method)
{
    int elapsed{ 0 };
    switch (method) 
    {
    case TICK_COUNT:
        elapsed = CheckForTickCount(func);
        break;
    case LOCAL_TIME:
        elapsed = CheckForLocalTime(func);
        break;
    case QPC:
        elapsed = CheckForQPC(func);
        break;
    }

    
    return elapsed;
}