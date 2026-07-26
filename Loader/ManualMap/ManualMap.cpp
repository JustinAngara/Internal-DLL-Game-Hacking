#include "ManualMap.h"

const char szDllFile[] = "C:\\Users\\justi\\C++ Code\\Internal Base Starter\\ProcessHooker\\ProcessHooker\\out\\build\\x64-Release\\ProcessHooker.dll";
const char szProc[]    = "notepad++.exe";



DWORD SR_ManualMap(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet)
{
	
	
	
	return ManualMap(hTargetProc, szDllFile);
}



DWORD ManualMap(HANDLE hProc, const char* szDllFile)
{
	BYTE* pSrcData                      = nullptr;
	IMAGE_NT_HEADERS* pOldNtHeader      = nullptr;
	IMAGE_OPTIONAL_HEADER* interior_ptr = nullptr;
	IMAGE_FILE_HEADER* pOldFileHeader   = nullptr;
	BYTE* pTargetBase                   = nullptr;

	DWORD dwCheck = 0;
	if (!GetFileAttributesA(szDllFile))
	{
		return SR_ERR_FILE_DOESNT_EXIST;
	}



}


// will utlize start routine
int pseudoEntryTest()
{
	return 0;
}