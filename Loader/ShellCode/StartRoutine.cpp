#include "StartRoutine.h"
#include "../ManualMap/ManualMap.h"
#include "Shellcode.h"


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
	case LM_ManualMap:
		dwRet = SR_ManualMap(hTargetProc, pRoutine, pArg, LastWin32Error, RemoteRet);
		break;
	default:
		dwRet = SR_ERR_INVALID_LAUNCH_METHOD;
		break;
	}
	return dwRet;
}

DWORD SR_NtCreateThreadEx(HANDLE hTargetProc, f_Routine* pRoutine, void* pArg, DWORD& LastWin32Error, UINT_PTR& RemoteRet)
{
#ifdef _WIN64
	BYTE SHELLCODE[sizeof(ShellCode::X64::B_NtCreateThread)];
	std::memcpy(SHELLCODE, ShellCode::X64::B_NtCreateThread, sizeof(SHELLCODE));

#else
	BYTE SHELLCODE[sizeof(ShellCode::X64::B_NtCreateThread)]; // this is the exact same so its fine to not be x86
	std::memcpy(SHELLCODE, ShellCode::X64::B_NtCreateThread, sizeof(SHELLCODE));
#endif

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
	*reinterpret_cast<void**>      (SHELLCODE + 0x00) = pArg;
	*reinterpret_cast<f_Routine**> (SHELLCODE + 0x08) = pRoutine;

	DWORD FuncOffset = 0x10;
	BOOL bRet = WriteProcessMemory(hTargetProc, pMem, SHELLCODE, sizeof(SHELLCODE), nullptr);
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
#ifdef _WIN64
	BYTE SHELLCODE[sizeof(ShellCode::X64::B_HijackThread)];
	std::memcpy(SHELLCODE, ShellCode::X64::B_HijackThread, sizeof(SHELLCODE));
#else
	BYTE SHELLCODE[sizeof(ShellCode::X86::B_HijackThread)]; 
	std::memcpy(SHELLCODE, ShellCode::X86::B_HijackThread, sizeof(SHELLCODE));
#endif


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



	DWORD FuncOffset	  = 0x08;
	DWORD CheckByteOffset = 0x03 + FuncOffset;

	DWORD dwLoRIP = (DWORD)( OldContext.Rip & 0xFFFFFFFF );
	DWORD dwHiRIP = (DWORD( (OldContext.Rip) >> 0x20) & 0xFFFFFFFF );

	*reinterpret_cast<DWORD*>(SHELLCODE + 0x07 + FuncOffset) = dwLoRIP;
	*reinterpret_cast<DWORD*>(SHELLCODE + 0x0F + FuncOffset) = dwHiRIP;

	*reinterpret_cast<void**>(SHELLCODE + 0x21 + FuncOffset) = pRoutine;
	*reinterpret_cast<void**>(SHELLCODE + 0x2B + FuncOffset) = pArg;

	OldContext.Rip = reinterpret_cast<UINT_PTR>(pCodecave) + FuncOffset;

#else

	

	DWORD FuncOffset		= 0x04;
	DWORD CheckByteOffset	= 0x02 + FuncOffset;

	*reinterpret_cast<DWORD*>(SHELLCODE + 0x06 + FuncOffset) = OldContext.Eip;

	*reinterpret_cast<void**>(SHELLCODE + 0x0F + FuncOffset) = pArg;
	*reinterpret_cast<void**>(SHELLCODE + 0x14 + FuncOffset) = pRoutine;

	*reinterpret_cast<void**>(SHELLCODE + 0x1C + FuncOffset) = pCodecave;
	*reinterpret_cast<BYTE**>(SHELLCODE + 0x26 + FuncOffset) = reinterpret_cast<BYTE*>(pCodecave) + CheckByteOffset;

	OldContext.Eip = reinterpret_cast<DWORD>(pCodecave) + FuncOffset;

#endif

	if (!WriteProcessMemory(hTargetProc, pCodecave, SHELLCODE, sizeof(SHELLCODE), nullptr))
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
#ifdef _WIN64
	BYTE SHELLCODE[sizeof(ShellCode::X64::B_SetWindowsHook)];
	std::memcpy(SHELLCODE, ShellCode::X64::B_SetWindowsHook, sizeof(SHELLCODE));
#else
	BYTE SHELLCODE[sizeof(ShellCode::X86::B_SetWindowsHook)]; 
	std::memcpy(SHELLCODE, ShellCode::X86::B_SetWindowsHook, sizeof(SHELLCODE));
#endif




	void* pCodecave = VirtualAllocEx(hTargetProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pCodecave)
	{
		LastWin32Error = GetLastError();

		return SR_SWHEX_ERR_CANT_ALLOC_MEM;
	}

	void* pCallNextHookEx = GetProcAddressEx(hTargetProc, TEXT("user32.dll"), "CallNextHookEx");
	if (!pCallNextHookEx)
	{
		VirtualFreeEx(hTargetProc, pCodecave, 0, MEM_RELEASE);

		return SR_SWHEX_ERR_CNHEX_MISSING;
	}

#ifdef _WIN64

	

	DWORD CodeOffset = 0x18;
	DWORD CheckByteOffset = 0x22 + CodeOffset;

	*reinterpret_cast<void**>(SHELLCODE + 0x00) = pArg;
	*reinterpret_cast<void**>(SHELLCODE + 0x08) = pRoutine;
	*reinterpret_cast<void**>(SHELLCODE + 0x10) = pCallNextHookEx;

#else

	
	DWORD CodeOffset	  = 0x08;
	DWORD CheckByteOffset = 0x14 + CodeOffset;

	*reinterpret_cast<void**>(SHELLCODE + 0x00) = pArg;
	*reinterpret_cast<void**>(SHELLCODE + 0x04) = pRoutine;

	*reinterpret_cast<DWORD*>(SHELLCODE + 0x0F + CodeOffset) = reinterpret_cast<DWORD>(pCallNextHookEx) - (reinterpret_cast<DWORD>(pCodecave) + 0x0E + CodeOffset) - 5;
	*reinterpret_cast<void**>(SHELLCODE + 0x18 + CodeOffset) = pCodecave;

#endif

	if (!WriteProcessMemory(hTargetProc, pCodecave, SHELLCODE, sizeof(SHELLCODE), nullptr))
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
#ifdef _WIN64
	BYTE SHELLCODE[sizeof(ShellCode::X64::B_QueueUserAPC)];
	std::memcpy(SHELLCODE, ShellCode::X64::B_QueueUserAPC, sizeof(SHELLCODE));
#else
	BYTE SHELLCODE[sizeof(ShellCode::X86::B_QueueUserAPC)]; 
	std::memcpy(SHELLCODE, ShellCode::X86::B_QueueUserAPC, sizeof(SHELLCODE));
#endif



	void* pCodecave = VirtualAllocEx(hTargetProc, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pCodecave)
	{
		LastWin32Error = GetLastError();

		return SR_QUAPC_ERR_CANT_ALLOC_MEM;
	}

#ifdef _WIN64


	DWORD CodeOffset = 0x18;

	*reinterpret_cast<void**>(SHELLCODE + 0x08) = pArg;
	*reinterpret_cast<void**>(SHELLCODE + 0x10) = pRoutine;

#else

	
	DWORD CodeOffset = 0x0C;

	*reinterpret_cast<void**>(SHELLCODE + 0x04) = pArg;
	*reinterpret_cast<void**>(SHELLCODE + 0x08) = pRoutine;

#endif

	BOOL bRet = WriteProcessMemory(hTargetProc, pCodecave, SHELLCODE, sizeof(SHELLCODE), nullptr);
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
