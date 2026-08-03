#include <stdio.h>
#include <windows.h>
#include "AntiDbg.h"
#include "../Vars.h"


static NtQueryInformationThread_t fnNtQueryInformationThread = NULL;
static NtSetInformationThread_t   fnNtSetInformationThread   = NULL;


HANDLE AntiDbg::RunThreadEx(LPTHREAD_START_ROUTINE ThreadEx)
{
    DWORD dwThreadId = 0;
    HANDLE hThread = CreateThread(NULL, 0, ThreadEx, NULL, 0, &dwThreadId);
    if (!hThread) 
    {
        return NULL;
    }

    return hThread;
}

HANDLE AntiDbg::RunHideThreadDebugger(LPTHREAD_START_ROUTINE ThreadMain)
{
    DWORD dwThreadId = 0;
    HANDLE hThread = CreateThread(NULL, 0, ThreadMain, NULL, 0, &dwThreadId);
    if (!hThread) 
    {
        return NULL;
    }

    HMODULE hDLL = LoadLibraryW(L"ntdll.dll");
    if (!hDLL)
    {
        return hThread;
    }

    fnNtQueryInformationThread = (NtQueryInformationThread_t)GetProcAddress(hDLL, "NtQueryInformationThread");
    fnNtSetInformationThread   = (NtSetInformationThread_t)GetProcAddress(hDLL, "NtSetInformationThread");
    if (!fnNtQueryInformationThread || !fnNtSetInformationThread) return hThread;

    BOOLEAN bHidden = FALSE;
    ULONG   lRet = 0;
    fnNtSetInformationThread(hThread, ThreadHideFromDebugger, NULL, 0);
    fnNtQueryInformationThread(hThread, ThreadHideFromDebugger, &bHidden, sizeof(bHidden), &lRet);
    printf("Thread is hidden: %s\n", bHidden ? "Yes" : "No");

    return hThread;             
}

DWORD64 AntiDbg::CheckForTickCount(Func func)
{

    DWORD64 tickReference = GetTickCount();
    printf("\nbefore: %d\n", tickReference);

    func(); 

    DWORD64 currentTick = GetTickCount();
    printf("\nafter: %d\n", currentTick);

    DWORD64 elapsedTime = currentTick - tickReference;
    printf("\ndifference: %u", elapsedTime);


    return elapsedTime;
}
 

DWORD64 AntiDbg::CheckForLocalTime(Func func) 
{
    SYSTEMTIME sysStart, sysend;
    FILETIME fStart, fEnd;
    ULARGE_INTEGER uiStart, uiEnd;

    GetLocalTime(&sysStart);

    func();

    GetLocalTime(& sysend);

    if (!SystemTimeToFileTime(&sysend, &fEnd))
    {
        return 0;
    }
    if (!SystemTimeToFileTime(&sysStart, &fStart))
    {
        return 0;
    }

    uiStart.LowPart = fStart.dwLowDateTime;
    uiStart.HighPart = fStart.dwHighDateTime;
    uiEnd.LowPart = fEnd.dwLowDateTime;
    uiEnd.HighPart = fEnd.dwHighDateTime;

    return (((uiEnd.QuadPart - uiStart.QuadPart)*100)/1000000); // filetime conversion to ms
} 


DWORD64 AntiDbg::CheckForQPC(Func func) 
{
    LARGE_INTEGER start, end, frequency;
    QueryPerformanceCounter(&start);
    QueryPerformanceFrequency(&frequency);

    func();

    QueryPerformanceCounter(&end);
    return (end.QuadPart - start.QuadPart)*1000/frequency.QuadPart;
}


DWORD64 AntiDbg::CheckForRDTSC(Func func) {
    DWORD64 start = __rdtsc();

    func(); 

    DWORD64 end = __rdtsc();

    return (end - start);
}


DWORD64 AntiDbg::CheckForDebugger(Func func, AntiDbgMethod method)
{
    DWORD64 elapsed{ 0 };
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
    case RDTSC:
        elapsed = CheckForRDTSC(func);
        if (elapsed >= MAX_ALLOWED_CYCLES) { /* suspicious */ }
        break;
    }

    
    return elapsed;
}