#include <Windows.h>
#include <fstream>
#include "../ShellCode/StartRoutine.h"

using f_LoadLibraryA    = HINSTANCE (WINAPI*)(const char* lpLibFilename);
using f_GetProcAddress  = UINT_PTR  (WINAPI*)(HINSTANCE hModule, const char* lpProcName);
using f_DLL_ENTRY_POINT = BOOL      (WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);



struct MANUAL_MAPPING
{
	f_LoadLibraryA   pLoadLibraryA;
	f_GetProcAddress pGetProcAddress;
	HINSTANCE        hMod;
};


DWORD SR_ManualMap(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet);
DWORD ManualMap(HANDLE hProc, const char* szDllFile);