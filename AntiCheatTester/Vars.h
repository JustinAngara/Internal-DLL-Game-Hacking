#pragma once
#include <Windows.h>

typedef LONG NTSTATUS;
#define ThreadHideFromDebugger 0x11

typedef NTSTATUS (WINAPI *NtQueryInformationThread_t)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef NTSTATUS (WINAPI *NtSetInformationThread_t)(HANDLE, ULONG, PVOID, ULONG);

using Func = void(*)();   

enum ChildState
{
    UNKNOWN,
    ALIVE,
    DEAD,
    ERR
};

// anti cheat, anti dbg, obfuscation tester, random stuff3
constexpr int MAX_TIME_TO_SLEEP           = 5000;
constexpr int MAX_TIME_TO_ATTACH_DEBUGGER = 5000;
constexpr int MAX_TIME_TO_DO_FUNC_CALL    = 2500;

