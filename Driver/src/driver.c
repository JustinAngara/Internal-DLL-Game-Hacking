#include <ntddk.h>
#include <wdf.h>
#include "procdriver_ioctl.h"

DRIVER_INITIALIZE                  DriverEntry;
EVT_WDF_DRIVER_UNLOAD              EvtDriverUnload;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL EvtIoDeviceControl;

#define LOG(fmt, ...) \
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ProcDriver: " fmt "\n", __VA_ARGS__)

static NTSTATUS CreateControlDevice(_In_ WDFDRIVER Driver)
{
    NTSTATUS            status;
    PWDFDEVICE_INIT     deviceInit;
    WDFDEVICE           device;
    WDF_IO_QUEUE_CONFIG queueConfig;

    DECLARE_CONST_UNICODE_STRING(sddl, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");
    DECLARE_CONST_UNICODE_STRING(deviceName, PROCDRIVER_DEVICE_NAME);
    DECLARE_CONST_UNICODE_STRING(symLink, PROCDRIVER_SYMLINK_NAME);

    deviceInit = WdfControlDeviceInitAllocate(Driver, &sddl);
    if (deviceInit == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = WdfDeviceInitAssignName(deviceInit, &deviceName);
    if (!NT_SUCCESS(status)) {
        WdfDeviceInitFree(deviceInit);
        return status;
    }

    status = WdfDeviceCreate(&deviceInit, WDF_NO_OBJECT_ATTRIBUTES, &device);
    if (!NT_SUCCESS(status)) {
        WdfDeviceInitFree(deviceInit);
        return status;
    }

    status = WdfDeviceCreateSymbolicLink(device, &symLink);
    if (!NT_SUCCESS(status)) {
        WdfObjectDelete(device);
        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchSequential);
    queueConfig.EvtIoDeviceControl = EvtIoDeviceControl;

    status = WdfIoQueueCreate(device, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        WdfObjectDelete(device);
        return status;
    }

    WdfControlFinishInitializing(device);
    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    NTSTATUS          status;
    WDF_DRIVER_CONFIG config;
    WDFDRIVER         driver;

    WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
    config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
    config.EvtDriverUnload  = EvtDriverUnload;

    status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, &driver);
    if (!NT_SUCCESS(status)) {
        LOG("WdfDriverCreate failed 0x%08X", status);
        return status;
    }

    status = CreateControlDevice(driver);
    if (!NT_SUCCESS(status)) {
        LOG("CreateControlDevice failed 0x%08X", status);
        return status;
    }

    LOG("loaded (KMDF %u.%u)", KMDF_VERSION_MAJOR, KMDF_VERSION_MINOR);
    return STATUS_SUCCESS;
}

VOID EvtDriverUnload(_In_ WDFDRIVER Driver)
{
    UNREFERENCED_PARAMETER(Driver);
    LOG("unloaded%s", "");
}

VOID EvtIoDeviceControl(
    _In_ WDFQUEUE   Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t     OutputBufferLength,
    _In_ size_t     InputBufferLength,
    _In_ ULONG      IoControlCode)
{
    static const CHAR greeting[] = "Hello from ProcDriver";
    NTSTATUS  status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG_PTR bytes  = 0;
    PVOID     outBuf;

    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    switch (IoControlCode) {
    case IOCTL_PROCDRIVER_HELLO:
        status = WdfRequestRetrieveOutputBuffer(Request, sizeof(greeting), &outBuf, NULL);
        if (NT_SUCCESS(status)) {
            RtlCopyMemory(outBuf, greeting, sizeof(greeting));
            bytes = sizeof(greeting);
        }
        break;
    default:
        break;
    }

    WdfRequestCompleteWithInformation(Request, status, bytes);
}
