#include "GetProcAddress.h"


HINSTANCE GetModuleHandlEx(HANDLE hTargetProc, const TCHAR* lpModuleName)
{
	MODULEENTRY32 ME32{ 0 };
	ME32.dwSize = sizeof(ME32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(hTargetProc));

	if (hSnap == INVALID_HANDLE_VALUE)
	{
		while (GetLastError() == ERROR_BAD_LENGTH)
		{
			hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(hTargetProc));
			if (hSnap != INVALID_HANDLE_VALUE) break;
		}
	}
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	BOOL bRet = Module32First(hSnap, &ME32);

	do
	{
		if (!_tcsicmp(lpModuleName, ME32.szModule))
		{
			break;
		}
		bRet = Module32Next(hSnap, &ME32);
	} while (bRet);

	CloseHandle(hSnap);
	if (!bRet)
	{
		return NULL;
	}

	ME32.hModule;
}
void* GetProcAddressEx(HANDLE hTargetProc, const TCHAR* lpModuleName, const char* lpProcName)
{
	BYTE* modBase reinterpret_cast<void*>(GetModuleHandlEx(hTargetProc, lpModuleName));
	BYTE* pe_header = new BYTE[0x1000];
	if (!modBase || !pe_header)
	{
		return nullptr;
	}

	if (!ReadProcessMemory(hTargetProc, pe_header, 0x1000, nullptr))
	{
		delete[] pe_header;
		return nullptr;
	}

	auto* pNT = reinterpret_cast<IMAGE_NT_HEADERS*>(pe_header + reinterpret_cast<IMAGE_DOS_HEADER*>(pe_header)->e_lfanew);
	auto* pExportEntry = &pNT->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (!pExportEntry->Size)
	{
		delete[] pe_header;
		return nullptr;
	}

	BYTE* export_data = new BYTE[pExportEntry->Size];
	if (!export_data)
	{
		delete[] pe_header;
		return nullptr;
	}



	if (!ReadProcessMemory(hTargetProc, modBase + pExportEntry->VirtualAddress, export_data, pExportEntry->Size))
	{
		delete[] export_data;
		delete[] pe_header;
		return nullptr;
	}

	BYTE* localBase = export_data - pExportEntry->VirtualAddress;



	return nullptr;
}
