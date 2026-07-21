#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <tchar.h>

HINSTANCE GetModuleHandlEx(HANDLE hTargetProc, const TCHAR* lpModuleName);
void* GetProcAddressEx(HANDLE hTargetProc, const TCHAR* lpModuleName, const char* lpProcName);
