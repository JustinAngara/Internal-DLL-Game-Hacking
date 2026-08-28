#pragma once
// scan for any sort of modified memory regions
// patch changes with original bytes
#include <Windows.h>

namespace MemoryPatcher
{
	PVOID ScanPatch(LPCSTR funcName);
	size_t RunPatch(PVOID hookedFunc, PVOID originalFunc, DWORD funcSize);
	void PatchAll();
}