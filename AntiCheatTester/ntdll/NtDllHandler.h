#pragma once
#include <Windows.h>
#include <TlHelp32.h>

namespace NT
{
    int* GetProcList();
    int* GetDriverList();
}

typedef LONG KPRIORITY, *PKPRIORITY;

typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef const UNICODE_STRING* PCUNICODE_STRING;

typedef struct _CLIENT_ID
{
    PVOID UniqueProcess;
    PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

// must be defined BEFORE SYSTEM_PROCESS_INFORMATION
typedef struct _SYSTEM_THREAD_INFORMATION
{
    FILETIME  ProcessorTime;
    FILETIME  UserTime;
    FILETIME  CreateTime;
    ULONG     WaitTime;
#ifdef _WIN64
    ULONG     pad1;
#endif
    PVOID     StartAddress;
    CLIENT_ID Client_Id;
    LONG      CurrentPriority;
    LONG      BasePriority;
    ULONG     ContextSwitchesPerSec;
    ULONG     ThreadState;
    ULONG     ThreadWaitReason;
    ULONG     pad2;
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _VM_COUNTERS
{
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG  PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_PROCESS_INFORMATION
{
    ULONG                    NextOffset;
    ULONG                    ThreadCount;
    LARGE_INTEGER            WorkingSetPrivateSize;
    ULONG                    HardFaultCount;
    ULONG                    NumberOfThreadsHighWatermark;
    ULONGLONG                CycleTime;
    FILETIME                 CreateTime;
    FILETIME                 UserTime;
    FILETIME                 KernelTime;
    UNICODE_STRING           ImageName;
    LONG                     BasePriority;
#ifdef _WIN64
    ULONG                    pad1;
#endif
    ULONG                    ProcessId;
#ifdef _WIN64
    ULONG                    pad2;
#endif
    ULONG                    InheritedFromProcessId;
#ifdef _WIN64
    ULONG                    pad3;
#endif
    ULONG                    HandleCount;
    ULONG                    SessionId;
    ULONG_PTR                UniqueProcessKey;
    VM_COUNTERS              VirtualMemoryCounters;
    ULONG_PTR                PrivatePageCount;
    IO_COUNTERS              IoCounters;
    SYSTEM_THREAD_INFORMATION ThreadInfos[1];
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _SYSTEM_MODULE_ENTRY
{
    HANDLE Section;
    PVOID  MappedBase;
    PVOID  ImageBase;
    ULONG  ImageSize;
    ULONG  Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR  FullPathName[256];
} SYSTEM_MODULE_ENTRY, *PSYSTEM_MODULE_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION
{
    ULONG             Count;
    SYSTEM_MODULE_ENTRY Module[1];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _SYSTEM_PROCESS_ID_INFORMATION
{
    HANDLE         ProcessId;
    UNICODE_STRING ImageName;
} SYSTEM_PROCESS_ID_INFORMATION, *PSYSTEM_PROCESS_ID_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation                = 0,
    SystemPerformanceInformation          = 2,
    SystemTimeOfDayInformation            = 3,
    SystemProcessInformation              = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemModuleInformation               = 11,
    SystemInterruptInformation            = 23,
    SystemExceptionInformation            = 33,
    SystemRegistryQuotaInformation        = 37,
    SystemLookasideInformation            = 45,
    SystemCodeIntegrityInformation        = 103,
    SystemPolicyInformation               = 134,
} SYSTEM_INFORMATION_CLASS;

typedef LONG NTSTATUS;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;



// SYSTEM PROC INFO
typedef NTSTATUS(NTAPI* pNtQuerySystemInformation)(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

typedef NTSTATUS(NTAPI* pNtQueryInformationProcess)(
    HANDLE ProcessHandle,
    ULONG ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
);

// EXECUTION CONTROL
typedef NTSTATUS(NTAPI* pNtYieldExecution)();

// MEMORY MANAGEMENT
typedef NTSTATUS(NTAPI* pNtAllocateVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
);

typedef NTSTATUS(NTAPI* pNtProtectVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    PSIZE_T RegionSize,
    ULONG NewProtect,
    PULONG OldProtect
);

typedef NTSTATUS(NTAPI* pNtReadVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T NumberOfBytesToRead,
    PSIZE_T NumberOfBytesRead
);

typedef NTSTATUS(NTAPI* pNtWriteVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T NumberOfBytesToWrite,
    PSIZE_T NumberOfBytesWritten
);

typedef NTSTATUS(NTAPI* pNtFreeVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    PSIZE_T RegionSize,
    ULONG FreeType
);

typedef NTSTATUS(NTAPI* pNtQueryVirtualMemory)(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    ULONG MemoryInformationClass,
    PVOID MemoryInformation,
    SIZE_T MemoryInformationLength,
    PSIZE_T ReturnLength
);

// PROC thread maangement
typedef NTSTATUS(NTAPI* pNtOpenProcess)(
    PHANDLE ProcessHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, 
    PCLIENT_ID ClientId
);

typedef NTSTATUS(NTAPI* pNtOpenThread)(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, 
    PCLIENT_ID ClientId
);

typedef NTSTATUS(NTAPI* pNtCreateThreadEx)(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    PVOID ObjectAttributes,
    HANDLE ProcessHandle,
    PVOID StartRoutine,
    PVOID Argument,
    ULONG CreateFlags,
    ULONG_PTR ZeroBits,
    SIZE_T StackSize,
    SIZE_T MaximumStackSize,
    PVOID AttributeList
);

typedef NTSTATUS(NTAPI* pNtTerminateProcess)(
    HANDLE ProcessHandle,
    NTSTATUS ExitStatus
);

// HANDLES
typedef NTSTATUS(NTAPI* pNtClose)(
    HANDLE Handle
);