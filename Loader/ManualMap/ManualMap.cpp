#include "ManualMap.h"


DWORD SR_ManualMap(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet)
{
	
	return ManualMap(hTargetProc, szDllFile);
}



DWORD ManualMap(HANDLE hProc, const char* szDllFile)
{
	BYTE* pSrcData                       = nullptr;
	IMAGE_NT_HEADERS* pOldNtHeader       = nullptr;
	IMAGE_OPTIONAL_HEADER* interior_ptr  = nullptr;
	IMAGE_OPTIONAL_HEADER* pOldOptHeader = nullptr;
	IMAGE_FILE_HEADER* pOldFileHeader    = nullptr;
	BYTE* pTargetBase                    = nullptr;

	DWORD dwCheck = 0;
	if (!GetFileAttributesA(szDllFile))
	{
		return SR_ERR_FILE_DOESNT_EXIST;
	}

	std::ifstream File(szDllFile, std::ios::binary | std::ios::ate);
	if (File.fail())
	{
		return SR_ERR_OPEN_FILE;
	}

	auto fileSize = File.tellg();
	if (fileSize < 0x1000)
	{
		File.close();
		return SR_ERR_FILE_SIZE;
	}

	pSrcData = new BYTE[static_cast<UINT_PTR>(fileSize)];
	if (!pSrcData)
	{
		File.close();
		return SR_MANUAL_ERR_CANT_ALLOC_MEM;
	}
	

	File.seekg(0,std::ios::beg);
	File.read(reinterpret_cast<char*>(pSrcData), fileSize);
	File.close();
	

	if (reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_magic != 0x5A4D) // 'MZ'
	{
		printf("invalid file\n");
		delete[] pSrcData;
		return false;
	}

	pOldNtHeader   = reinterpret_cast<IMAGE_NT_HEADERS*>(pSrcData + reinterpret_cast<IMAGE_DOS_HEADER*>(pSrcData)->e_lfanew);
	pOldOptHeader  = &pOldNtHeader->OptionalHeader;
	pOldFileHeader = &pOldNtHeader->FileHeader;



#ifdef _WIN64
	if (pOldFileHeader->Machine != IMAGE_FILE_MACHINE_AMD64)
	{
		printf("invalid platform");
		delete[] pSrcData;
		return false;
	}

#else
	if (pOldFileHeader->Machine != IMAGE_FILE_MACHINE_I386)
	{
		printf("invalid platform");
		delete[] pSrcData;
		return false;
	}
#endif

	pTargetBase = reinterpret_cast<BYTE*>( VirtualAllocEx(hProc, reinterpret_cast<void*>(pOldOptHeader->ImageBase), pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	if (pTargetBase)
	{
		pTargetBase = reinterpret_cast<BYTE*>( VirtualAllocEx(hProc, nullptr, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
		if (!pTargetBase)
		{
			printf("Memory Allocation Failed");
			delete[] pSrcData;
			return false;
		}
	}



}


// will utlize start routine
int pseudoEntryTest()
{
	return 0;
}