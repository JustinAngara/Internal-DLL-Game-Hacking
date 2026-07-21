#include "StartRoutine.h"


DWORD SR_NtCreateThreadEx(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet);
DWORD SR_HijackThread    (HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet);
DWORD SR_SetWindowsHookEx(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet);
DWORD SR_QueueUserAPC    (HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet);

DWORD StartRoutine(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, LAUNCH_METHOD Method, DWORD& LastWin32Error, UINT_PTR& RemoteRet)
{
	DWORD dwRet = 0;
	switch (Method)
	{
	case LM_NtCreateThreadEx:
		dwRet = SR_NtCreateThreadEx(hTargetProc, pRoutine, pArg, LastWin32Error, RemoteRet);
		break;

	case LM_HijackThread:
		dwRet = SR_HijackThread(hTargetProc, pRoutine, pArg, LastWin32Error, RemoteRet);
		break;

	case LM_SetWindowsHookEx:
		dwRet = SR_SetWindowsHookEx(hTargetProc, pRoutine, pArg, LastWin32Error, RemoteRet);
		break;

	case LM_QueueUserAPC:
		dwRet = SR_QueueUserAPC(hTargetProc, pRoutine, pArg, LastWin32Error, RemoteRet);
		break;

	default:
		dwRet = SR_ERR_INVALID_LAUNCH_METHOD;
		break;
	}
	return 0;
}

DWORD SR_NtCreateThreadEx(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& Out)
{
	auto p_NtCreateThreadEx = reinterpret_cast<f_NtCreateThreadEx>( GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtCreateThreadEx"));

	if (!p_NtCreateThreadEx)
	{
		LastWin32Error = GetLastError();
		return SR_NTCTE_ERR_NTCTE_MISSING;
	}

	void* pMem = VirtualAllocEx(hTargetProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pMem)
	{
		LastWin32Error = GetLastError();
		return SR_NTCTE_ERR_CANT_ALLOC_MEM;
	}


#ifdef _WIN64
	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // - 0x10   -> argument / returned value
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // - 0x08   -> pRoutine

		0x48, 0x8B, 0xC1,                                   // + 0x00   -> mov rax, rcx
		0x48, 0x8B, 0x08,                                   // + 0x03   -> mov rcx, [rax]

		0x48, 0x83, 0xEC, 0x28,                             // + 0x06   -> sub rsp, 0x28
		0xFF, 0x50, 0x08,                                   // + 0x0A   -> call qword ptr [rax + 0x08]
		0x48, 0x83, 0xC4, 0x28,                             // + 0x0D   -> add rsp, 0x28

		0x48, 0x8D, 0x0D, 0xD8, 0xFF, 0xFF, 0xFF,           // + 0x11   -> lea rcx, [pCodecave]
		0x48, 0x89, 0x01,                                   // + 0x18   -> mov [rcx], rax
		0x48, 0x31, 0xC0,                                   // + 0x1B   -> xor rax, rax

		0xC3                                                // + 0x1E   -> ret
	}; // SIZE = 0x1F (+ 0x10)

	*reinterpret_cast<void**>      (Shellcode+0x00)   = pArg;
	*reinterpret_cast<f_Routine**> (Shellcode + 0x08) = pRoutine;

	DWORD FuncOffset = 0x10;
	BOOL bRet = WriteProcessMemory(hTargetProc, pMem, Shellcode, sizeof(Shellcode), nullptr);
	if (!bRet)
	{
		LastWin32Error = GetLastError();
		VirtualFreeEx(hTargetProc, pMem, 0, MEM_RELEASE);
		return SR_NTCTE_ERR_WPM_FAIL;
	}

	void* pRemoteArg = pMem;
	void* pRemoteFunc = reinterpret_cast<BYTE*>(pMem) + FuncOffset;
	
	HANDLE hThread = nullptr;
	NTSTATUS ntRet;

#else

#endif

	return 0;
}

DWORD SR_HijackThread(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& Out)
{
	return 0;
}

DWORD SR_SetWindowsHookEx(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& Out)
{
	return 0;
}

DWORD SR_QueueUserAPC(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& Out)
{
	return 0;
}
