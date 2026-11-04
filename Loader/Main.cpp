#include "ShellCode/StartRoutine.h"
#include "ManualMap/ManualMap.h"

#include <TlHelp32.h>
#include <iostream>
#define DLL_PATH_X86 TEXT("ProcessHooker.dll")
#define DLL_PATH_X64 TEXT("C:\\Users\\justi\\C++ Code\\Internal Base Starter\\ProcessHooker\\ProcessHooker\\out\\build\\x64-Release\\ProcessHooker.dll")
#define PROCESS_NAME_X64 TEXT("notepad++.exe")
#define PROCESS_NAME_X86 TEXT("notepad++.exe")

#define LOAD_LIBRARY_NAME_A "LoadLibraryA"
#define LOAD_LIBRARY_NAME_W	"LoadLibraryW"

#ifdef UNICODE
#define LOAD_LIBRARY_NAME LOAD_LIBRARY_NAME_W
#else
#define LOAD_LIBRARY_NAME LOAD_LIBRARY_NAME_A
#endif

#ifdef _WIN64
#define DLL_PATH		DLL_PATH_X64
#define PROCESS_NAME	PROCESS_NAME_X64
#else
#define DLL_PATH		DLL_PATH_X86
#define PROCESS_NAME	PROCESS_NAME_X86
#endif

int ManualMapEntry(const TCHAR* szProc)
{
	// print proc
	_tprintf(_T("%s\n"), szProc);

	PROCESSENTRY32 PE32{ 0 };
	PE32.dwSize = sizeof(PE32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		DWORD Err = GetLastError();
		printf("CreateToolhelp32Snapshot failed: 0x%X\n", Err);
		system("PAUSE");
		return 0;
	}

	DWORD PID = 0;
	BOOL bRet = Process32First(hSnap, &PE32);
	while (bRet)
	{
		if (_tcsicmp(PE32.szExeFile, szProc) == 0)   
		{
			PID = PE32.th32ProcessID;
			break;
		}
		bRet = Process32Next(hSnap, &PE32);
	}

	CloseHandle(hSnap);

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (!hProc)
	{
		DWORD Err = GetLastError();
		printf("OpenProcess failed: 0x%X\n", Err);
		system("PAUSE");
		return 0;
	}


	if (!ManualMap(hProc, szDllFile))
	{
		CloseHandle(hProc);
		printf("Something went wrong FeelsBadMan\n");
		system("PAUSE");
		return 0;
	}

	CloseHandle(hProc);

	return 0;
}

HANDLE GetProcessByName(const TCHAR* szProcName, DWORD dwDesiredAccess = PROCESS_ALL_ACCESS)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		return nullptr;
	}

	PROCESSENTRY32 PE32{ 0 };
	PE32.dwSize = sizeof(PE32);

	BOOL bRet = Process32First(hSnap, &PE32);
	while (bRet)
	{
		if (!_tcsicmp(PE32.szExeFile, szProcName))
		{
			break;
		}

		bRet = Process32Next(hSnap, &PE32);
	}

	CloseHandle(hSnap);

	if (!bRet)
	{
		return nullptr;
	}

	return OpenProcess(dwDesiredAccess, FALSE, PE32.th32ProcessID);
}

bool InjectDll(const TCHAR* szProcess, const TCHAR* szPath, LAUNCH_METHOD Method)
{

	if (Method == LM_ManualMap)
	{
		// manual map special
		return ManualMapEntry( PROCESS_NAME );
	}


	HANDLE hProc = GetProcessByName(szProcess);
	if (!hProc)
	{
		DWORD dwErr = GetLastError();
		printf("OpenProcess failed: 0x%08X\n", dwErr);

		return false;
	}

	auto len = _tcslen(szPath) * sizeof(TCHAR);
	void * pArg = VirtualAllocEx(hProc, nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pArg)
	{
		DWORD dwErr = GetLastError();
		printf("VirtualAllocEx failed: 0x%08X\n", dwErr);

		CloseHandle(hProc);

		return false;
	}

	BOOL bRet = WriteProcessMemory(hProc, pArg, szPath, len, nullptr);
	if (!bRet)
	{
		DWORD dwErr = GetLastError();
		printf("WriteProcessMemory failed: 0x%08X\n", dwErr);

		VirtualFreeEx(hProc, pArg, 0, MEM_RELEASE);
		CloseHandle(hProc);

		return false;
	}

	f_Routine* p_LoadLibrary = reinterpret_cast<f_Routine*>(GetProcAddressEx(hProc, TEXT("kernel32.dll"), LOAD_LIBRARY_NAME));
	if (!p_LoadLibrary)
	{		
		printf("Can't find LoadLibrary\n");

		VirtualFreeEx(hProc, pArg, 0, MEM_RELEASE);
		CloseHandle(hProc);

		return false;
	}

	UINT_PTR hDllOut = 0;
	DWORD last_error = 0;
	DWORD dwErr = StartRoutine(hProc, p_LoadLibrary, pArg, Method, last_error, hDllOut);

	if (Method != LM_QueueUserAPC)
	{
		VirtualFreeEx(hProc, pArg, 0, MEM_RELEASE);
	}

	CloseHandle(hProc);

	if (dwErr)
	{
		printf("StartRoutine failed: 0x%08X\n", dwErr);
		printf("	LastWin32Error: 0x%08X\n", last_error);

		return false;
	}

	printf("Success! LoadLibrary returned 0x%p\n", reinterpret_cast<void*>(hDllOut));

	return true;
}

int main()
{

	InjectDll(PROCESS_NAME_X64, DLL_PATH, LM_ManualMap);


	printf("Press enter to exit.\n");
	std::cin.get();

	return 0;	
}