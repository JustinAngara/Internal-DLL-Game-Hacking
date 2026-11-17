#pragma once
// scan for any sort of modified memory regions
// patch changes with original bytes
#include <Windows.h>

namespace MemoryPatcher
{
    // important functions to patching
    const char* functionsToPatch[] = {
        // important stuff
        "NtReadVirtualMemory",
        "NtWriteVirtualMemory",
        "NtProtectVirtualMemory",
        "NtQuerySystemInformation",
        "NtSetSystemInformation",
        "NtOpenProcess",
        "NtAllocateVirtualMemory",
        "NtFreeVirtualMemory",
        // time bs
        "NtGetTickCount",
        "NtQueryPerformanceCounter",
        "NtQuerySystemTime",
        "RtlTimeFieldsToTime",
        // APC
        "NtQueueApcThread"

    };

	PVOID ScanPatch(LPCSTR funcName);
	size_t RunPatch(PVOID hookedFunc, PVOID originalFunc, DWORD funcSize);
	void PatchAll();
}