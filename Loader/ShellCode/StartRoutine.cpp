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
	return dwRet;
}

DWORD SR_NtCreateThreadEx(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet)
{
	auto p_NtCreateThreadEx = reinterpret_cast<f_NtCreateThreadEx>(GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtCreateThreadEx"));

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

	*reinterpret_cast<void**>      (Shellcode + 0x00) = pArg;
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
	NTSTATUS ntRet = p_NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, nullptr, hTargetProc, pRemoteFunc, pRemoteArg, 0, 0, 0, 0, nullptr);
	if (NT_FAIL(ntRet) || !hThread)
	{
		LastWin32Error = ntRet;
		VirtualFreeEx(hTargetProc, pMem, 0, MEM_RELEASE);
		return SR_NTCTE_ERR_NTCTE_FAIL;
	}

	DWORD dwWaitRet = WaitForSingleObject(hThread, SR_REMOTE_TIMEOUT);
	if (dwWaitRet != WAIT_OBJECT_0)
	{
		LastWin32Error = GetLastError();
		TerminateThread(hThread, 0);
		CloseHandle(hThread);
		VirtualFreeEx(hTargetProc, pMem, 0, MEM_RELEASE);
		return SR_NTCTE_ERR_TIMEOUT;
	}
	CloseHandle(hThread);

	bRet = ReadProcessMemory(hTargetProc, pMem, &RemoteRet, sizeof(RemoteRet), nullptr);

	VirtualFreeEx(hTargetProc, pMem, 0, MEM_RELEASE);
	if (!bRet)
	{
		LastWin32Error = GetLastError();
		return SR_NTCTE_ERR_RPM_FAIL;
	}

#else
	// x86
	HANDLE hThread = nullptr;
	NTSTATUS ntRet = p_NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, nullptr, hTargetProc, pRoutine, pArg, 0, 0, 0, 0, nullptr);
	if (NT_FAIL(ntRet) || !hThread)
	{
		LastWin32Error = ntRet;
		VirtualFreeEx(hTargetProc, pMem, 0, MEM_RELEASE);
		return SR_NTCTE_ERR_NTCTE_FAIL;
	}

	DWORD dwWaitRet = WaitForSingleObject(hThread, SR_REMOTE_TIMEOUT);
	if (!dwWaitRet)
	{
		LastWin32Error = GetLastError();
		TerminateThread(hThread, 0);
		CloseHandle(hThread);
		VirtualFreeEx(hTargetProc, pMem, 0, MEM_RELEASE);
		return SR_NTCTE_ERR_TIMEOUT;
	}

	DWORD dwRemoteRet = 0;
	BOOL bRet = GetExitCodeThread(hThread, &dwRemoteRet);
	if (!bRet)
	{
		LastWin32Error = GetLastError();
		CloseHandle(hThread);
		return SR_NTCTE_ERR_RPM_FAIL;
	}

	RemoteRet = dwRemoteRet;
	CloseHandle(hThread);


#endif

	return SR_ERR_SUCCESS;
}

DWORD SR_HijackThread(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet)
{
	THREADENTRY32 TE32{ 0 };
	TE32.dwSize = sizeof(TE32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetProcessId(hTargetProc));
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		LastWin32Error = GetLastError();

		return SR_HT_ERR_TH32_FAIL;
	}

	DWORD dwTargetPID = GetProcessId(hTargetProc);
	DWORD ThreadID = 0;

	BOOL bRet = Thread32First(hSnap, &TE32);
	if (!bRet)
	{
		LastWin32Error = GetLastError();

		CloseHandle(hSnap);

		return SR_HT_ERR_T32FIRST_FAIL;
	}

	do
	{
		if (TE32.th32OwnerProcessID == dwTargetPID)
		{
			ThreadID = TE32.th32ThreadID; 
			break;
		}

		bRet = Thread32Next(hSnap, &TE32);
	} while (bRet);

	if (!ThreadID)
	{
		return SR_HT_ERR_NO_THREADS;
	}

	HANDLE hThread = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, ThreadID);
	if (!hThread)
	{
		LastWin32Error = GetLastError();

		return SR_HT_ERR_OPEN_THREAD_FAIL;
	}

	if (SuspendThread(hThread) == (DWORD)-1)
	{
		LastWin32Error = GetLastError();

		CloseHandle(hThread);

		return SR_HT_ERR_SUSPEND_FAIL;
	}

	CONTEXT OldContext{ 0 };
	OldContext.ContextFlags = CONTEXT_CONTROL;

	if (!GetThreadContext(hThread, &OldContext))
	{
		LastWin32Error = GetLastError();

		ResumeThread(hThread);
		CloseHandle(hThread);

		return SR_HT_ERR_GET_CONTEXT_FAIL;
	}

	void* pCodecave = VirtualAllocEx(hTargetProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pCodecave)
	{
		LastWin32Error = GetLastError();

		ResumeThread(hThread);
		CloseHandle(hThread);

		return SR_HT_ERR_CANT_ALLOC_MEM;
	}

#ifdef _WIN64

	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,							// - 0x08			-> returned value

		0x48, 0x83, 0xEC, 0x08,													// + 0x00			-> sub rsp, 0x08

		0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00,								// + 0x04 (+ 0x07)	-> mov [rsp], RipLowPart
		0xC7, 0x44, 0x24, 0x04, 0x00, 0x00, 0x00, 0x00,							// + 0x0B (+ 0x0F)	-> mov [rsp + 0x04], RipHighPart

		0x50, 0x51, 0x52, 0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53,		// + 0x13			-> push r(a/c/d)x / r(8 - 11)
		0x9C,																	// + 0x1E			-> pushfq

		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,				// + 0x1F (+ 0x21)	-> mov rax, pRoutine
		0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,				// + 0x29 (+ 0x2B)	-> mov rcx, pArg

		0x48, 0x83, 0xEC, 0x20,													// + 0x33			-> sub rsp, 0x20
		0xFF, 0xD0,																// + 0x37			-> call rax
		0x48, 0x83, 0xC4, 0x20,													// + 0x39			-> add rsp, 0x20

		0x48, 0x8D, 0x0D, 0xB4, 0xFF, 0xFF, 0xFF,								// + 0x3D			-> lea rcx, [pCodecave]
		0x48, 0x89, 0x01,														// + 0x44			-> mov [rcx], rax

		0x9D,																	// + 0x47			-> popfq
		0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58, 0x5A, 0x59, 0x58,		// + 0x48			-> pop r(11-8) / r(d/c/a)x

		0xC6, 0x05, 0xA9, 0xFF, 0xFF, 0xFF, 0x00,								// + 0x53			-> mov byte ptr[$ - 0x57], 0

		0xC3																	// + 0x5A			-> ret
	}; // SIZE = 0x5B (+ 0x08)

	DWORD FuncOffset	  = 0x08;
	DWORD CheckByteOffset = 0x03 + FuncOffset;

	DWORD dwLoRIP = (DWORD)( OldContext.Rip & 0xFFFFFFFF );
	DWORD dwHiRIP = (DWORD( (OldContext.Rip) >> 0x20) & 0xFFFFFFFF );

	*reinterpret_cast<DWORD*>(Shellcode + 0x07 + FuncOffset) = dwLoRIP;
	*reinterpret_cast<DWORD*>(Shellcode + 0x0F + FuncOffset) = dwHiRIP;

	*reinterpret_cast<void**>(Shellcode + 0x21 + FuncOffset) = pRoutine;
	*reinterpret_cast<void**>(Shellcode + 0x2B + FuncOffset) = pArg;

	OldContext.Rip = reinterpret_cast<UINT_PTR>(pCodecave) + FuncOffset;

#else

	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00,						// - 0x04 (pCodecave)	-> returned value							;buffer to store returned value (eax)

		0x83, 0xEC, 0x04,							// + 0x00				-> sub esp, 0x04							;prepare stack for ret
		0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00,	// + 0x03 (+ 0x06)		-> mov [esp], OldEip						;store old eip as return address

		0x50, 0x51, 0x52,							// + 0x0A				-> psuh e(a/c/d)							;save e(a/c/d)x
		0x9C,										// + 0x0D				-> pushfd									;save flags register

		0xB9, 0x00, 0x00, 0x00, 0x00,				// + 0x0E (+ 0x0F)		-> mov ecx, pArg							;load pArg into ecx
		0xB8, 0x00, 0x00, 0x00, 0x00,				// + 0x13 (+ 0x14)		-> mov eax, pRoutine

		0x51,										// + 0x18				-> push ecx									;push pArg
		0xFF, 0xD0,									// + 0x19				-> call eax									;call target function

		0xA3, 0x00, 0x00, 0x00, 0x00,				// + 0x1B (+ 0x1C)		-> mov dword ptr[pCodecave], eax			;store returned value

		0x9D,										// + 0x20				-> popfd									;restore flags register
		0x5A, 0x59, 0x58,							// + 0x21				-> pop e(d/c/a)								;restore e(d/c/a)x

		0xC6, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,	// + 0x24 (+ 0x26)		-> mov byte ptr[pCodecave + 0x06], 0x00		;set checkbyte to 0

		0xC3										// + 0x2B				-> ret										;return to OldEip
	}; // SIZE = 0x2C (+ 0x04)

	DWORD FuncOffset		= 0x04;
	DWORD CheckByteOffset	= 0x02 + FuncOffset;

	*reinterpret_cast<DWORD*>(Shellcode + 0x06 + FuncOffset) = OldContext.Eip;

	*reinterpret_cast<void**>(Shellcode + 0x0F + FuncOffset) = pArg;
	*reinterpret_cast<void**>(Shellcode + 0x14 + FuncOffset) = pRoutine;

	*reinterpret_cast<void**>(Shellcode + 0x1C + FuncOffset) = pCodecave;
	*reinterpret_cast<BYTE**>(Shellcode + 0x26 + FuncOffset) = reinterpret_cast<BYTE*>(pCodecave) + CheckByteOffset;

	OldContext.Eip = reinterpret_cast<DWORD>(pCodecave) + FuncOffset;

#endif

	if (!WriteProcessMemory(hTargetProc, pCodecave, Shellcode, sizeof(Shellcode), nullptr))
	{
		LastWin32Error = GetLastError();

		ResumeThread(hThread);
		CloseHandle(hThread);
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_HT_ERR_WPM_FAIL;
	}

	if (!SetThreadContext(hThread, &OldContext))
	{
		LastWin32Error = GetLastError();

		ResumeThread(hThread);
		CloseHandle(hThread);
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_HT_ERR_SET_CONTEXT_FAIL;
	}

	if (ResumeThread(hThread) == (DWORD)-1)
	{
		LastWin32Error = GetLastError();

		CloseHandle(hThread);
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_HT_ERR_RESUME_FAIL;
	}

	CloseHandle(hThread);

	DWORD Timer = GetTickCount();
	BYTE CheckByte = 1;

	do
	{
		ReadProcessMemory(hTargetProc, reinterpret_cast<BYTE*>(pCodecave) + CheckByteOffset, &CheckByte, 1, nullptr);

		if (GetTickCount() - Timer > SR_REMOTE_TIMEOUT)
		{
			return SR_HT_ERR_TIMEOUT;
		}

		Sleep(10);
	} while (CheckByte != 0);

	ReadProcessMemory(hTargetProc, pCodecave, &RemoteRet, sizeof(RemoteRet), nullptr);
	VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);


	return SR_ERR_SUCCESS;
}


DWORD SR_SetWindowsHookEx(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet)
{
	void * pCodecave = VirtualAllocEx(hTargetProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pCodecave)
	{
		LastWin32Error = GetLastError();

		return SR_SWHEX_ERR_CANT_ALLOC_MEM;
	}

	void * pCallNextHookEx = GetProcAddressEx(hTargetProc, TEXT("user32.dll"), "CallNextHookEx");
	if (!pCallNextHookEx)
	{
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_SWHEX_ERR_CNHEX_MISSING;
	}

#ifdef _WIN64

	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// - 0x18	-> pArg / returned value / rax	;buffer
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// - 0x10	-> pRoutine						;pointer to target function
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// - 0x08	-> CallNextHookEx				;pointer to CallNextHookEx

		0x55,											// + 0x00	-> push rbp						;save important registers
		0x54,											// + 0x01	-> push rsp
		0x53,											// + 0x02	-> push rbx

		0x48, 0x8D, 0x1D, 0xDE, 0xFF, 0xFF, 0xFF,		// + 0x03	-> lea rbx, [pArg]				;load pointer into rbx

		0x48, 0x83, 0xEC, 0x20,							// + 0x0A	-> sub rsp, 0x20				;reserve stack
		0x4D, 0x8B, 0xC8,								// + 0x0E	-> mov r9,r8					;set up arguments for CallNextHookEx
		0x4C, 0x8B, 0xC2,								// + 0x11	-> mov r8, rdx
		0x48, 0x8B, 0xD1,								// + 0x14	-> mov rdx,rcx
		0xFF, 0x53, 0x10,								// + 0x17	-> call [rbx + 0x10]			;call CallNextHookEx
		0x48, 0x83, 0xC4, 0x20,							// + 0x1A	-> add rsp, 0x20				;update stack

		0x48, 0x8B, 0xC8,								// + 0x1E	-> mov rcx, rax					;copy retval into rcx

		0xEB, 0x00,										// + 0x21	-> jmp $ + 0x02					;jmp to next instruction
		0xC6, 0x05, 0xF8, 0xFF, 0xFF, 0xFF, 0x18,		// + 0x23	-> mov byte ptr[$ - 0x01], 0x1A	;hotpatch jmp above to skip shellcode

		0x48, 0x87, 0x0B,								// + 0x2A	-> xchg [rbx], rcx				;store CallNextHookEx retval, load pArg
		0x48, 0x83, 0xEC, 0x20,							// + 0x2D	-> sub rsp, 0x20				;reserve stack
		0xFF, 0x53, 0x08,								// + 0x31	-> call [rbx + 0x08]			;call pRoutine
		0x48, 0x83, 0xC4, 0x20,							// + 0x34	-> add rsp, 0x20				;update stack

		0x48, 0x87, 0x03,								// + 0x38	-> xchg [rbx], rax				;store pRoutine retval, restore CallNextHookEx retval

		0x5B,											// + 0x3B	-> pop rbx						;restore important registers
		0x5C,											// + 0x3C	-> pop rsp
		0x5D,											// + 0x3D	-> pop rbp

		0xC3											// + 0x3E	-> ret							;return
	}; // SIZE = 0x3F (+ 0x18)

	DWORD CodeOffset = 0x18;
	DWORD CheckByteOffset = 0x22 + CodeOffset;

	*reinterpret_cast<void**>(Shellcode + 0x00) = pArg;
	*reinterpret_cast<void**>(Shellcode + 0x08) = pRoutine;
	*reinterpret_cast<void**>(Shellcode + 0x10) = pCallNextHookEx;

#else

	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00,			// - 0x08				-> pArg						;pointer to argument
		0x00, 0x00, 0x00, 0x00,			// - 0x04				-> pRoutine					;pointer to target function

		0x55,							// + 0x00				-> push ebp					;x86 stack frame creation
		0x8B, 0xEC,						// + 0x01				-> mov ebp, esp

		0xFF, 0x75, 0x10,				// + 0x03				-> push [ebp + 0x10]		;push CallNextHookEx arguments
		0xFF, 0x75, 0x0C,				// + 0x06				-> push [ebp + 0x0C] 
		0xFF, 0x75, 0x08, 				// + 0x09				-> push [ebp + 0x08]
		0x6A, 0x00,						// + 0x0C				-> push 0x00
		0xE8, 0x00, 0x00, 0x00, 0x00,	// + 0x0E (+ 0x0F)		-> call CallNextHookEx		;call CallNextHookEx

		0xEB, 0x00,						// + 0x13				-> jmp $ + 0x02				;jmp to next instruction

		0x50,							// + 0x15				-> push eax					;save eax (CallNextHookEx retval)
		0x53,							// + 0x16				-> push ebx					;save ebx (non volatile)

		0xBB, 0x00, 0x00, 0x00, 0x00,	// + 0x17 (+ 0x18)		-> mov ebx, pArg			;move pArg (pCodecave) into ebx
		0xC6, 0x43, 0x1C, 0x14,			// + 0x1C				-> mov [ebx + 0x1C], 0x17	;hotpatch jmp above to skip shellcode

		0xFF, 0x33,						// + 0x20				-> push [ebx]				;push pArg (__stdcall)

		0xFF, 0x53, 0x04,				// + 0x22				-> call [ebx + 0x04]		;call target function

		0x89, 0x03,						// + 0x25				-> mov [ebx], eax			;store returned value

		0x5B,							// + 0x27				-> pop ebx					;restore old ebx
		0x58,							// + 0x28				-> pop eax					;restore eax (CallNextHookEx retval)

		0x5D,							// + 0x29				-> pop ebp					;restore ebp
		0xC2, 0x0C, 0x00				// + 0x2A				-> ret 0x000C				;return
	}; // SIZE = 0x3D (+ 0x08)

	DWORD CodeOffset	  = 0x08;
	DWORD CheckByteOffset = 0x14 + CodeOffset;

	*reinterpret_cast<void**>(Shellcode + 0x00) = pArg;
	*reinterpret_cast<void**>(Shellcode + 0x04) = pRoutine;

	*reinterpret_cast<DWORD*>(Shellcode + 0x0F + CodeOffset) = reinterpret_cast<DWORD>(pCallNextHookEx) - (reinterpret_cast<DWORD>(pCodecave) + 0x0E + CodeOffset) - 5;
	*reinterpret_cast<void**>(Shellcode + 0x18 + CodeOffset) = pCodecave;

#endif

	if (!WriteProcessMemory(hTargetProc, pCodecave, Shellcode, sizeof(Shellcode), nullptr))
	{
		LastWin32Error = GetLastError();

		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_SWHEX_ERR_WPM_FAIL;
	}

	static EnumWindowsCallback_Data data;
	data.m_HookData.clear();

	data.m_pHook	= reinterpret_cast<HOOKPROC>(reinterpret_cast<BYTE*>(pCodecave) + CodeOffset);
	data.m_PID		= GetProcessId(hTargetProc);
	data.m_hModule	= GetModuleHandle(TEXT("user32.dll"));

	WNDENUMPROC EnumWindowsCallback = [](HWND hWnd, LPARAM) -> BOOL
		{
			DWORD winPID = 0;
			DWORD winTID = GetWindowThreadProcessId(hWnd, &winPID);
			if (winPID == data.m_PID)
			{
				TCHAR szWindow[MAX_PATH]{ 0 };
				if (IsWindowVisible(hWnd) && GetWindowText(hWnd, szWindow, MAX_PATH))
				{
					if (GetClassName(hWnd, szWindow, MAX_PATH) && _tcscmp(szWindow, TEXT("ConsoleWindowClass")))
					{
						HHOOK hHook = SetWindowsHookEx(WH_CALLWNDPROC, data.m_pHook, data.m_hModule, winTID);
						if (hHook)
						{
							data.m_HookData.push_back({ hHook, hWnd });
						}
					}
				}
			}

			return TRUE;
		};

	if (!EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&data)))
	{
		LastWin32Error = GetLastError();

		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_SWHEX_ERR_ENUM_WND_FAIL;
	}

	if (data.m_HookData.empty())
	{
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_SWHEX_ERR_NO_WINDOWS;
	}

	HWND hForegroundWnd = GetForegroundWindow();
	for (auto i : data.m_HookData)
	{
		SetForegroundWindow(i.m_hWnd);
		SendMessageA(i.m_hWnd, WM_KEYDOWN, VK_SPACE, 0);
		Sleep(10);
		SendMessageA(i.m_hWnd, WM_KEYUP, VK_SPACE, 0);
		UnhookWindowsHookEx(i.m_hHook);
	}
	SetForegroundWindow(hForegroundWnd);

	DWORD Timer = GetTickCount();
	BYTE CheckByte = 0;

	do
	{
		ReadProcessMemory(hTargetProc, reinterpret_cast<BYTE*>(pCodecave) + CheckByteOffset, &CheckByte, 1, nullptr);

		if (GetTickCount() - Timer > SR_REMOTE_TIMEOUT)
		{
			return SR_SWHEX_ERR_TIMEOUT;
		}

		Sleep(10);

	} while (!CheckByte);


	ReadProcessMemory(hTargetProc, pCodecave, &RemoteRet, sizeof(RemoteRet), nullptr);

	VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

	return SR_ERR_SUCCESS;
}

DWORD SR_QueueUserAPC(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet)
{
	void* pCodecave = VirtualAllocEx(hTargetProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pCodecave)
	{
		LastWin32Error = GetLastError();

		return SR_QUAPC_ERR_CANT_ALLOC_MEM;
	}

#ifdef _WIN64

	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// - 0x18	-> returned value							;buffer to store returned value
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// - 0x10	-> pArg										;buffer to store argument
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// - 0x08	-> pRoutine									;pointer to the rouinte to call

		0xEB, 0x00,											// + 0x00	-> jmp $+0x02								;jump to the next instruction

		0x48, 0x8B, 0x41, 0x10,								// + 0x02	-> mov rax, [rcx + 0x10]					;move pRoutine into rax
		0x48, 0x8B, 0x49, 0x08,								// + 0x06	-> mov rcx, [rcx + 0x08]					;move pArg into rcx

		0x48, 0x83, 0xEC, 0x28,								// + 0x0A	-> sub rsp, 0x28							;reserve stack
		0xFF, 0xD0,											// + 0x0E	-> call rax									;call pRoutine
		0x48, 0x83, 0xC4, 0x28,								// + 0x10	-> add rsp, 0x28							;update stack

		0x48, 0x85, 0xC0,									// + 0x14	-> test rax, rax							;check if rax indicates success/failure
		0x74, 0x11,											// + 0x17	-> je pCodecave + 0x2A						;jmp to ret if routine failed

		0x48, 0x8D, 0x0D, 0xC8, 0xFF, 0xFF, 0xFF,			// + 0x19	-> lea rcx, [pCodecave]						;load pointer to codecave into rcx
		0x48, 0x89, 0x01,									// + 0x20	-> mov [rcx], rax							;store returned value

		0xC6, 0x05, 0xD7, 0xFF, 0xFF, 0xFF, 0x28,			// + 0x23	-> mov byte ptr[pCodecave + 0x18], 0x28		;hot patch jump to skip shellcode

		0xC3												// + 0x2A	-> ret										;return
	}; // SIZE = 0x2B (+ 0x10)

	DWORD CodeOffset = 0x18;

	*reinterpret_cast<void**>(Shellcode + 0x08) = pArg;
	*reinterpret_cast<void**>(Shellcode + 0x10) = pRoutine;

#else

	BYTE Shellcode[] = 
	{
		0x00, 0x00, 0x00, 0x00, // - 0x0C	-> returned value					;buffer to store returned value
		0x00, 0x00, 0x00, 0x00, // - 0x08	-> pArg								;buffer to store argument
		0x00, 0x00, 0x00, 0x00, // - 0x04	-> pRoutine							;pointer to the routine to call

		0x55,					// + 0x00	-> push ebp							;x86 stack frame creation
		0x8B, 0xEC,				// + 0x01	-> mov ebp, esp

		0xEB, 0x00,				// + 0x03	-> jmp pCodecave + 0x05 (+ 0x0C)	;jump to next instruction

		0x53,					// + 0x05	-> push ebx							;save ebx
		0x8B, 0x5D, 0x08,		// + 0x06	-> mov ebx, [ebp + 0x08]			;move pCodecave into ebx (non volatile)

		0xFF, 0x73, 0x04,		// + 0x09	-> push [ebx + 0x04]				;push pArg on stack
		0xFF, 0x53, 0x08,		// + 0x0C	-> call dword ptr[ebx + 0x08]		;call pRoutine

		0x85, 0xC0,				// + 0x0F	-> test eax, eax					;check if eax indicates success/failure
		0x74, 0x06,				// + 0x11	-> je pCodecave + 0x19 (+ 0x0C)		;jmp to cleanup if routine failed

		0x89, 0x03,				// + 0x13	-> mov [ebx], eax					;store returned value
		0xC6, 0x43, 0x10, 0x15, // + 0x15	-> mov byte ptr [ebx + 0x10], 0x15	;hot patch jump to skip shellcode

		0x5B,					// + 0x19	-> pop ebx							;restore old ebx

		0x5D,					// + 0x1A	-> pop ebp							;restore ebp
		0xC2, 0x04, 0x00		// + 0x1B	-> ret 0x0004						;return
	}; // SIZE = 0x1E (+ 0x0C)

	DWORD CodeOffset = 0x0C;

	*reinterpret_cast<void**>(Shellcode + 0x04) = pArg;
	*reinterpret_cast<void**>(Shellcode + 0x08) = pRoutine;

#endif

	BOOL bRet = WriteProcessMemory(hTargetProc, pCodecave, Shellcode, sizeof(Shellcode), nullptr);
	if (!bRet)
	{
		LastWin32Error = GetLastError();

		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_QUAPC_ERR_WPM_FAIL;
	}

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetProcessId(hTargetProc));
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		LastWin32Error = GetLastError();

		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_QUAPC_ERR_TH32_FAIL;
	}

	DWORD TargetPID = GetProcessId(hTargetProc);
	bool APCQueued = false;
	PAPCFUNC pShellcode = reinterpret_cast<PAPCFUNC>(reinterpret_cast<BYTE*>(pCodecave) + CodeOffset);

	THREADENTRY32 TE32{ 0 };
	TE32.dwSize = sizeof(TE32);

	bRet = Thread32First(hSnap, &TE32);
	if (!bRet)
	{		
		LastWin32Error = GetLastError();

		CloseHandle(hSnap);
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_QUAPC_ERR_T32FIRST_FAIL;
	}

	do
	{
		if (TE32.th32OwnerProcessID == TargetPID)
		{
			HANDLE hThread = OpenThread(THREAD_SET_CONTEXT, FALSE, TE32.th32ThreadID);
			if (hThread)
			{
				if (QueueUserAPC(pShellcode, hThread, reinterpret_cast<ULONG_PTR>(pCodecave)))
				{
					APCQueued = true;
				}
				else
				{
					LastWin32Error = GetLastError();
				}

				CloseHandle(hThread);
			}		
		}

		bRet = Thread32Next(hSnap, &TE32);

	} while (bRet);

	CloseHandle(hSnap);

	if (!APCQueued)
	{
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_QUAPC_ERR_NO_APC_THREAD;
	}
	else
	{
		LastWin32Error = 0;
	}

	DWORD Timer = GetTickCount();
	RemoteRet = 0;

	do
	{
		ReadProcessMemory(hTargetProc, pCodecave, &RemoteRet, sizeof(RemoteRet), nullptr);

		if (GetTickCount() - Timer > SR_REMOTE_TIMEOUT)
		{
			return SR_SWHEX_ERR_TIMEOUT;
		}

		Sleep(10);

	} while (!RemoteRet);

	return SR_ERR_SUCCESS;
}
