/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.
    
Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"


NTSTATUS
MouseLikeTouchPadFilterCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    PDEVICE_CONTEXT deviceContext;
    WDFDEVICE device;
    NTSTATUS status;
    WDF_PNPPOWER_EVENT_CALLBACKS  pnpPowerCallbacks;

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
        return status;
    }

    deviceContext = DeviceGetContext(device);
    
    //
    // Initialize the context.
    //
    deviceContext->PrivateDeviceData = 0;
    deviceContext->FxDevice = device;
    //
    // Create a device interface so that applications can find and talk
    // to us.
    //
    status = WdfDeviceCreateDeviceInterface(
        device,
        &GUID_DEVINTERFACE_MouseLikeTouchPadFilter,
        NULL // ReferenceString
    );
    if (!NT_SUCCESS(status)) {
        RegDebug(deviceContext, L"WdfDeviceCreateDeviceInterface Failed", NULL, status);
        return status;
    }


    // Initialize the I/O Package and any Queues
    status = MouseLikeTouchPadFilterQueueInitialize(device);

   
    RegDebug(deviceContext, L"MouseLikeTouchPadFilterCreateDevice ok", NULL, status);
    return status;
}


NTSTATUS EvtDeviceD0Entry(WDFDEVICE Device, WDF_POWER_DEVICE_STATE PreviousState)
{

    NTSTATUS   status = STATUS_SUCCESS;
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(Device);
    RegDebug(pDevContext, L"EvtDeviceD0Entry", NULL, status);

    UNREFERENCED_PARAMETER(pDevContext);

    UNREFERENCED_PARAMETER(PreviousState);
    /////
    return STATUS_SUCCESS;
}


NTSTATUS EvtDeviceD0Exit(WDFDEVICE Device, WDF_POWER_DEVICE_STATE TargetState)
{
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(Device);
    RegDebug(pDevContext, L"EvtDeviceD0Exit", NULL, 0);

    UNREFERENCED_PARAMETER(pDevContext);
    UNREFERENCED_PARAMETER(TargetState);

    return STATUS_SUCCESS;
}
