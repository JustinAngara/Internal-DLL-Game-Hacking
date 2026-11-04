#include <Windows.h>
#include <fstream>
#include "../ShellCode/StartRoutine.h"


#define RELOC_FLAG32(RelInfo)((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo)((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)
#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG32
#endif



const char szDllFile[] = "C:\\Users\\justi\\C++ Code\\Internal Base Starter\\ProcessHooker\\ProcessHooker\\out\\build\\x64-Release\\ProcessHooker.dll";
const char szProc[]    = "notepad++.exe";


using f_LoadLibraryA    = HINSTANCE (WINAPI*)(const char* lpLibFilename);
using f_GetProcAddress  = UINT_PTR  (WINAPI*)(HINSTANCE hModule, const char* lpProcName);
using f_DLL_ENTRY_POINT = BOOL      (WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);



struct MANUAL_MAPPING_DATA
{
	f_LoadLibraryA   pLoadLibraryA;
	f_GetProcAddress pGetProcAddress;
	HINSTANCE        hMod;
};


DWORD SR_ManualMap(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet);
DWORD ManualMap(HANDLE hProc, const char* szDllFile);