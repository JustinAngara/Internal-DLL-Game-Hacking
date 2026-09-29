#pragma once

#define PROCDRIVER_DEVICE_NAME   L"\\Device\\ProcDriver"
#define PROCDRIVER_SYMLINK_NAME  L"\\DosDevices\\ProcDriver"
#define PROCDRIVER_USER_PATH     L"\\\\.\\ProcDriver"

#define IOCTL_PROCDRIVER_HELLO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
