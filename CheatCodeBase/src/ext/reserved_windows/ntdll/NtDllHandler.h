#pragma once

#include "ntdll.h"
#include <Windows.h>
namespace NtDllHandler
{
	PEB GetPEBExternal(HANDLE hProc);
	PEB* GetPEBInternal();
	LDR_DATA_TABLE_ENTRY* GetLDREntryInternal(const wchar_t* modName);
	char* GetModuleBaseAddressInternalPEB(const wchar_t* modName);
}