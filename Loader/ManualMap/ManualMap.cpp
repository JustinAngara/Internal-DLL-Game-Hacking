#include "ManualMap.h"


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


}


// will utlize start routine
int pseudoEntryTest()
{
	return 0;
}