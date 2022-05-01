/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"

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
    queueConfig.EvtIoInternalDeviceControl = MouseLikeTouchPadFilterEvtIoInternalDeviceControl;//经过测试该函数不会执行
    queueConfig.EvtIoStop = MouseLikeTouchPadFilterEvtIoStop;
    queueConfig.EvtIoRead = MouseLikeTouchPadFilterEvtIoRead;
    queueConfig.EvtIoDefault = MouseLikeTouchPadFilterEvtIoDefault;//经过测试该函数不会执行
    queueConfig.EvtIoWrite = MouseLikeTouchPadFilterEvtIoWrite;//经过测试该函数不会执行
    queueConfig.EvtIoResume = MouseLikeTouchPadFilterEvtIoResume;//经过测试该函数不会执行

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if(!NT_SUCCESS(status)) {
        return status;
    }

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
    //Filter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(device));//经过测试该函数使得手势操作有效，否则不执行ptp报告失效而鼠标报告有效


    switch (IoControlCode)
    {
    case IOCTL_HID_GET_COLLECTION_INFORMATION://经过测试会执行
        RegDebug(pDevContext, L"IOCTL_HID_GET_COLLECTION_INFORMATION", NULL, IoControlCode); 
        break;

    case IOCTL_HID_GET_COLLECTION_DESCRIPTOR://经过测试会执行
        RegDebug(pDevContext, L"IOCTL_HID_GET_COLLECTION_DESCRIPTOR", NULL, IoControlCode); 
        break;

    case IOCTL_HID_GET_PRODUCT_STRING://经过测试会执行
        break;

    case IOCTL_HID_GET_FEATURE://经过测试会执行
        break;

    default:
        break;
    }

    //其他的IoControlCode交给下层处理
    Filter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(device));
    return;
}


VOID
MouseLikeTouchPadFilterEvtIoInternalDeviceControl(
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

    MessageBox(NULL, L"失败终止", L"终止", 0);
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

    RegDebug(pDevContext, L"EvtIoRead runtimes", NULL, Status);

    Filter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(device));

};

VOID
MouseLikeTouchPadFilterEvtIoWrite(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request,
    _In_
    size_t Length
)
{
    //经过测试MouseLikeTouchPadFilterEvtIoWrite没有执行过
    UNREFERENCED_PARAMETER(Length);
    //MessageBox(NULL, L"失败终止", L"终止", 0);

    NTSTATUS Status = STATUS_SUCCESS;
    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(device);

    UNREFERENCED_PARAMETER(pDevContext);

    RegDebug(pDevContext, L"EvtIoWrite runtimes", NULL, Status);

    Filter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(device));

};


VOID
MouseLikeTouchPadFilterEvtIoDefault(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request
)
{
    //经过测试MouseLikeTouchPadFilterEvtIoDEFAULT没有执行过
    //MessageBox(NULL, L"失败终止", L"终止", 0);

    NTSTATUS Status = STATUS_SUCCESS;
    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(device);

    UNREFERENCED_PARAMETER(pDevContext);

    RegDebug(pDevContext, L"MouseLikeTouchPadFilterEvtIoDEFAULT runtimes", NULL, Status);

    Filter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(device));

};

VOID
MouseLikeTouchPadFilterEvtIoResume(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request
)
{
    //经过测试MouseLikeTouchPadFilterEvtIoResume没有执行过
    //MessageBox(NULL, L"失败终止", L"终止", 0);

    NTSTATUS Status = STATUS_SUCCESS;
    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    DEVICE_CONTEXT* pDevContext = DeviceGetContext(device);

    UNREFERENCED_PARAMETER(pDevContext);

    RegDebug(pDevContext, L"MouseLikeTouchPadFilterEvtIoResume runtimes", NULL, Status);

    Filter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(device));

};


VOID
Filter_DispatchPassThrough(
    _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target
)
{
    //RegDebug(pDevContext, L"Filter_DispatchPassThrough", NULL, 0);

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
        RegDebug(pDevContext, L"WdfRequestSend failed", NULL, status);
        WdfRequestComplete(Request, status);
    }

    RegDebug(pDevContext, L"Filter_DispatchPassThrough WdfRequestSend ok", 0, status);

    return;
}