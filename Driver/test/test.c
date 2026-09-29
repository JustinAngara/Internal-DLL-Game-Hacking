#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include "procdriver_ioctl.h"

int wmain(void)
{
    HANDLE h = CreateFileW(PROCDRIVER_USER_PATH, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        fwprintf(stderr, L"CreateFile %ls failed: %lu\n", PROCDRIVER_USER_PATH, GetLastError());
        return 1;
    }

    char  buf[128] = {0};
    DWORD bytes = 0;
    if (!DeviceIoControl(h, IOCTL_PROCDRIVER_HELLO, NULL, 0, buf, sizeof(buf) - 1, &bytes, NULL)) {
        fwprintf(stderr, L"DeviceIoControl failed: %lu\n", GetLastError());
        CloseHandle(h);
        return 1;
    }

    printf("%s\n", buf);
    CloseHandle(h);
    return 0;
}
