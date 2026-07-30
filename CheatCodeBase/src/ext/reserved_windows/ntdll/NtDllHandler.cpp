
#include "NtDllHandler.h"
#include "ntdll.h"
#include <Windows.h>
#include <TlHelp32.h>

// documentation
// char* modBase2 = GetModuleBaseAddressInternalPEB(L"ntdll.dll");
PEB NtDllHandler::GetPEBExternal(HANDLE hProc)
{
	PROCESS_BASIC_INFORMATION pbi;
	PEB peb = { 0 };

	HMODULE hNtdll = GetModuleHandleA("ntdll.dll");

	if (hNtdll == NULL) 
	{
		return peb; 
	}

	tNtQueryInformationProcess NtQueryInformationProcess =
		(tNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");

	if (NtQueryInformationProcess == NULL) 
	{
		return peb;
	}

	NTSTATUS status = NtQueryInformationProcess(hProc, ProcessBasicInformation, &pbi, sizeof(pbi), 0);
	if (NT_SUCCESS(status))
	{
		ReadProcessMemory(hProc, pbi.PebBaseAddress, &peb, sizeof(peb), 0);
	}

	return peb;
}


PEB* NtDllHandler::GetPEBInternal()
{
#ifdef _WIN64
	PEB* peb = (PEB*)__readgsqword(0x60);

#else
	PEB* peb = (PEB*)__readfsdword(0x30);
#endif

	return peb;
}

LDR_DATA_TABLE_ENTRY* NtDllHandler::GetLDREntryInternal(const wchar_t* modName)
{
	LDR_DATA_TABLE_ENTRY* modEntry = nullptr;

	PEB* peb = GetPEBInternal();

	LIST_ENTRY head = peb->Ldr->InMemoryOrderModuleList;

	LIST_ENTRY curr = head;

	for (auto curr = head; curr.Flink != &peb->Ldr->InMemoryOrderModuleList; curr = *curr.Flink)
	{
		LDR_DATA_TABLE_ENTRY* mod = (LDR_DATA_TABLE_ENTRY*)CONTAINING_RECORD(curr.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		if (mod->BaseDllName.Buffer)
		{
			if (_wcsicmp(modName, mod->BaseDllName.Buffer) == 0)
			{
				modEntry = mod;
				break;
			}
		}
	}
	return modEntry;
}

char* NtDllHandler::GetModuleBaseAddressInternalPEB(const wchar_t* modName)
{
	LDR_DATA_TABLE_ENTRY* modEntry = GetLDREntryInternal(modName);

	return (char*)modEntry->DllBase;
}

