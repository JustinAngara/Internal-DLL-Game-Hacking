#include "HideProcess.h"
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <winternl.h>
#include <iostream>
#include "ext/minhook/MinHook.h"

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

typedef struct _MY_SYSTEM_PROCESS_INFORMATION
{
    ULONG           NextEntryOffset;
    ULONG           NumberOfThreads;
    LARGE_INTEGER   Reserved[3];
    LARGE_INTEGER   CreateTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   KernelTime;
    UNICODE_STRING  ImageName;
    LONG            BasePriority;
    HANDLE          ProcessId;
    HANDLE          InheritedFromProcessId;
} MY_SYSTEM_PROCESS_INFORMATION, *PMY_SYSTEM_PROCESS_INFORMATION;

typedef NTSTATUS(WINAPI* PNT_QUERY_SYSTEM_INFORMATION)(
    __in       SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __inout    PVOID SystemInformation,
    __in       ULONG SystemInformationLength,
    __out_opt  PULONG ReturnLength
    );

PNT_QUERY_SYSTEM_INFORMATION OriginalNtQuerySystemInformation = nullptr;

NTSTATUS WINAPI HookedNtQuerySystemInformation(
    __in       SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __inout    PVOID                    SystemInformation,
    __in       ULONG                    SystemInformationLength,
    __out_opt  PULONG                   ReturnLength
)
{
    NTSTATUS status = OriginalNtQuerySystemInformation(
        SystemInformationClass,
        SystemInformation,
        SystemInformationLength,
        ReturnLength);

    if (SystemProcessInformation == SystemInformationClass && STATUS_SUCCESS == status)
    {
        PMY_SYSTEM_PROCESS_INFORMATION pCurrent = NULL;
        PMY_SYSTEM_PROCESS_INFORMATION pNext = (PMY_SYSTEM_PROCESS_INFORMATION)SystemInformation;

        do
        {
            pCurrent = pNext;
            pNext = (PMY_SYSTEM_PROCESS_INFORMATION)((PUCHAR)pCurrent + pCurrent->NextEntryOffset);

            // insert your dll or list of processes to hide from.
            if (pNext->ImageName.Buffer) /* &&
                !wcsncmp(pNext->ImageName.Buffer, L"cheatname.exe", pNext->ImageName.Length / sizeof(WCHAR))*/ 
            {
                if (!pNext->NextEntryOffset)
                    pCurrent->NextEntryOffset = 0;
                else
                    pCurrent->NextEntryOffset += pNext->NextEntryOffset;

                pNext = pCurrent;
            }
        } while (pCurrent->NextEntryOffset != 0);
    }
    return status;
}

void StartHook()
{
    // utilize logger
    void* pTarget = (void*)GetProcAddress(GetModuleHandle(L"ntdll"), "NtQuerySystemInformation");

    if (!pTarget)
    {
        MessageBox(NULL, L"Failed to get NtQuerySystemInformation", L"DEBUG", MB_OK);
        return;
    }

    if (MH_Initialize() != MH_OK)
    {
        MessageBox(NULL, L"MH_Initialize failed", L"DEBUG", MB_OK);
        return;
    }

    if (MH_CreateHook(pTarget, &HookedNtQuerySystemInformation,
        reinterpret_cast<LPVOID*>(&OriginalNtQuerySystemInformation)) != MH_OK)
    {
        MessageBox(NULL, L"MH_CreateHook failed", L"DEBUG", MB_OK);
        return;
    }

    if (MH_EnableHook(pTarget) != MH_OK)
    {
        MessageBox(NULL, L"MH_EnableHook failed", L"DEBUG", MB_OK); 
        return;
    }

    MessageBox(NULL, L"Hook installed successfully", L"DEBUG", MB_OK);
}

void HideProcess::Run()
{
    StartHook();
}