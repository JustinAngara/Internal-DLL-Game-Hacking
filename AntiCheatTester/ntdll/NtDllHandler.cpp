#include "NtDllHandler.h"
#include <stdlib.h>


extern "C" NTSTATUS NtQuerySystemInformation( SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID  SystemInformation, ULONG  SystemInformationLength, PULONG ReturnLength );

int* NT::GetProcList()
{
    int*  procList     = NULL;
    ULONG procListSize = 0;

    NtQuerySystemInformation(SystemProcessInformation, procList, procListSize, &procListSize);
    procList = (int*)realloc(procList, procListSize);
    NtQuerySystemInformation(SystemProcessInformation, procList, procListSize, &procListSize);

    return procList;
}

int* NT::GetDriverList()
{
    int*  driverList     = NULL;
    ULONG driverListSize = 0;

    NtQuerySystemInformation(SystemModuleInformation, driverList, driverListSize, &driverListSize);
    driverList = (int*)realloc(driverList, driverListSize);
    NtQuerySystemInformation(SystemModuleInformation, driverList, driverListSize, &driverListSize);

    return driverList;
}