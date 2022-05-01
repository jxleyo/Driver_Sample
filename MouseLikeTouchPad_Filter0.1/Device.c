/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"
#include "device.tmh"

NTSTATUS
MouseLikeTouchPadFilterCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    PDEVICE_CONTEXT deviceContext;
    WDFDEVICE device;
    NTSTATUS status;
    WDF_PNPPOWER_EVENT_CALLBACKS  pnpPowerCallbacks;

    RegDebug(L"MouseLikeTouchPadFilterCreateDevice start", NULL, 0);

    ////这是一个 过滤驱动
    WdfFdoInitSetFilter(DeviceInit);

    //设置电源回调函数
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

    //设备处于工作（供电D0状态）或者非工作状态
    pnpPowerCallbacks.EvtDeviceD0Entry = EvtDeviceD0Entry;
    pnpPowerCallbacks.EvtDeviceD0Exit = EvtDeviceD0Exit;

    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks); ///

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status)) {
        RegDebug(L"WdfDeviceCreate Failed", NULL, status);
        return status;
    }

    deviceContext = DeviceGetContext(device);

    //
    // Initialize the context.
    //
    deviceContext->PrivateDeviceData = 0;

    //
    // Create a device interface so that applications can find and talk
    // to us.
    //
    status = WdfDeviceCreateDeviceInterface(
        device,
        &GUID_DEVINTERFACE_MouseLikeTouchPadFilter,
        NULL // ReferenceString
    );
    if (NT_SUCCESS(status)) {
        RegDebug(L"WdfDeviceCreateDeviceInterface Failed", NULL, status);
        return status;
    }


    // Initialize the I/O Package and any Queues
    status = MouseLikeTouchPadFilterQueueInitialize(device);


    RegDebug(L"MouseLikeTouchPadFilterCreateDevice ok", NULL, status);
    return status;
}


NTSTATUS EvtDeviceD0Entry(WDFDEVICE Device, WDF_POWER_DEVICE_STATE PreviousState)
{

    NTSTATUS   status = STATUS_SUCCESS;
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(Device);
    RegDebug(L"EvtDeviceD0Entry", NULL, status);

    UNREFERENCED_PARAMETER(pDevContext);

    UNREFERENCED_PARAMETER(PreviousState);
    /////
    return STATUS_SUCCESS;
}


NTSTATUS EvtDeviceD0Exit(WDFDEVICE Device, WDF_POWER_DEVICE_STATE TargetState)
{
    RegDebug(L"EvtDeviceD0Exit", NULL, 0);
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(Device);

    UNREFERENCED_PARAMETER(pDevContext);
    UNREFERENCED_PARAMETER(TargetState);

    return STATUS_SUCCESS;
}
