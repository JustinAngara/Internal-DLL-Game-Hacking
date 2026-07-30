#pragma once
#include <Windows.h>

typedef LONG NTSTATUS;
#define ThreadHideFromDebugger 0x11

typedef NTSTATUS (WINAPI *NtQueryInformationThread_t)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef NTSTATUS (WINAPI *NtSetInformationThread_t)(HANDLE, ULONG, PVOID, ULONG);

using Func = void(*)();   