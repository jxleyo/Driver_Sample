/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"
#include "queue.tmh"

NTSTATUS
MouseLikeTouchPadFilterQueueInitialize(
    _In_ WDFDEVICE Device
    )
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG queueConfig;

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
        WdfIoQueueDispatchParallel
        );

    queueConfig.EvtIoDeviceControl = MouseLikeTouchPadFilterEvtIoDeviceControl;
    queueConfig.EvtIoStop = MouseLikeTouchPadFilterEvtIoStop;
    queueConfig.EvtIoRead = MouseLikeTouchPadFilterEvtIoRead;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if(!NT_SUCCESS(status)) {
        RegDebug(L"WdfIoQueueCreate Failed", NULL, status);
        return status;
    }

    RegDebug(L"MouseLikeTouchPadFilterQueueInitialize ok", NULL, status);
    return status;
}

VOID
MouseLikeTouchPadFilterEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
{
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    NTSTATUS status = STATUS_SUCCESS;
    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(device);
    UNREFERENCED_PARAMETER(pDevContext);

    UNREFERENCED_PARAMETER(status);
    UNREFERENCED_PARAMETER(IoControlCode);
    //Filter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(device));


    switch (IoControlCode)
    {

    case IOCTL_HID_GET_COLLECTION_INFORMATION:RegDebug(L"IOCTL_HID_GET_COLLECTION_INFORMATION", NULL, IoControlCode); break;

    case IOCTL_HID_GET_COLLECTION_DESCRIPTOR:RegDebug(L"IOCTL_HID_GET_COLLECTION_DESCRIPTOR", NULL, IoControlCode); break;

    default:
        RegDebug(L"STATUS_NOT_SUPPORTED", NULL, IoControlCode);
        status = STATUS_NOT_SUPPORTED;
    }

    WdfRequestComplete(Request, STATUS_SUCCESS);

    return;
}

VOID
MouseLikeTouchPadFilterEvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags
)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Request);
    UNREFERENCED_PARAMETER(ActionFlags);

    return;
}


VOID
MouseLikeTouchPadFilterEvtIoRead(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request,
    _In_
    size_t Length
)
{
    UNREFERENCED_PARAMETER(Length);

    NTSTATUS Status = STATUS_SUCCESS;
    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(device);

    UNREFERENCED_PARAMETER(pDevContext);

    RegDebug(L"EvtIoRead runtimes", NULL, Status);

    Filter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(device));

};


VOID
Filter_DispatchPassThrough(
    _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target
)
{
    //RegDebug(L"Filter_DispatchPassThrough", NULL, 0);

    WDF_REQUEST_SEND_OPTIONS options;
    BOOLEAN ret;
    NTSTATUS status = STATUS_SUCCESS;
    WDFDEVICE device = WdfIoQueueGetDevice(WdfRequestGetIoQueue(Request));
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(device);

    UNREFERENCED_PARAMETER(pDevContext);
    //
    // We are not interested in post processing the IRP so 
    // fire and forget.

    WDF_REQUEST_SEND_OPTIONS_INIT(&options,
        WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);//WDF_REQUEST_SEND_OPTION_TIMEOUT  //WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET

    //将I/O请求发送到下层设备前，做相应的数据处理//不设置完成例程时，下面这句可有可无
    //WdfRequestFormatRequestUsingCurrentType(Request);


    //将 I/O 请求发送到下层设备
    ret = WdfRequestSend(Request, Target, &options);//WDF_NO_SEND_OPTIONS
    if (ret == FALSE) {
        status = WdfRequestGetStatus(Request);
        RegDebug(L"WdfRequestSend failed", NULL, status);
        WdfRequestComplete(Request, status);
    }

    RegDebug(L"Filter_DispatchPassThrough WdfRequestSend ok", 0, status);

    return;
}