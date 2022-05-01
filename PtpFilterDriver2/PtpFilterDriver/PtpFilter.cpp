#include "PtpFilter.h"
extern "C" int _fltused = 0;

static int runtimes = 0;

/////
extern "C" NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS Status;
    WDF_DRIVER_CONFIG      config;
    WDF_OBJECT_ATTRIBUTES  DriverAttributes;

    WDF_OBJECT_ATTRIBUTES_INIT(&DriverAttributes);
    DriverAttributes.EvtCleanupCallback = EvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config, EvtDeviceAdd);

    Status = WdfDriverCreate(
        DriverObject,
        RegistryPath,
        &DriverAttributes,
        &config,
        WDF_NO_HANDLE);

    if (!NT_SUCCESS(Status)) {
        RegDebug(L"WdfDriverCreate failed", NULL, Status);
    }

    RegDebug(L"DriverEntry ok", NULL, 0);
    return Status;
}

void EvtDriverContextCleanup(IN WDFOBJECT DriverObject)
{
    RegDebug(L"EvtDriverContextCleanup", NULL, 0);
    UNREFERENCED_PARAMETER(DriverObject);
}


NTSTATUS
EvtDevicePrepareHardware(
    _In_ WDFDEVICE Device,
    _In_ WDFCMRESLIST ResourceList,
    _In_ WDFCMRESLIST ResourceListTranslated
)
{
    UNREFERENCED_PARAMETER(ResourceList);
    UNREFERENCED_PARAMETER(ResourceListTranslated);
    RegDebug(L"PtpFilterDevicePrepareHardware start", NULL, 0);

    PDEVICE_CONTEXT pDeviceContext = GetDeviceContext(Device);

    NTSTATUS Status = Hid_StartDevice(pDeviceContext);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterDevicePrepareHardware Hid_StartDevice failed", NULL, Status);
        goto Exit;
    }

    WDF_OBJECT_ATTRIBUTES  RequestAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&RequestAttributes);

    RequestAttributes.ParentObject = pDeviceContext->hDevice;
    Status = WdfRequestCreate(&RequestAttributes,
        pDeviceContext->IoTarget,
        &pDeviceContext->ReuseRequest);

    if (!NT_SUCCESS(Status)) 
    {
        RegDebug(L"PtpFilterDevicePrepareHardware WdfRequestCreate failed", NULL, Status);
        pDeviceContext->ReuseRequest = NULL;
        return Status;
    } 
    
    WDF_OBJECT_ATTRIBUTES_INIT(&RequestAttributes);
    RequestAttributes.ParentObject = pDeviceContext->hDevice;

    size_t REPORT_BUFFER_SIZE = pDeviceContext->REPORT_BUFFER_SIZE;
    pDeviceContext->ReportLength = (ULONG)REPORT_BUFFER_SIZE;

    if (!REPORT_BUFFER_SIZE)
    {
        RegDebug(L"PtpFilterDevicePrepareHardware InputReportByteLength is 0 ", NULL, 0);
        goto Exit;
    }

    Status = WdfMemoryCreate(
        &RequestAttributes,
        NonPagedPool,
        MOUHID_TAG,
        REPORT_BUFFER_SIZE,
        &pDeviceContext->RequestBuffer,
        &pDeviceContext->ReportBuffer);

    if (NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterDevicePrepareHardware WdfMemoryCreate failed", NULL, Status);

        pDeviceContext->RequestBuffer = NULL;
        return Status;

    }
    
    pDeviceContext->CurrentRequestReportBuffer = ExAllocatePoolWithTag(NonPagedPool, REPORT_BUFFER_SIZE, MOUHID_TAG);
    pDeviceContext->LastRequestReportBuffer = ExAllocatePoolWithTag(NonPagedPool, REPORT_BUFFER_SIZE, MOUHID_TAG);

    RegDebug(L"PtpFilterDevicePrepareHardware ok", NULL, Status);
    return STATUS_SUCCESS;
    
Exit:
    if ((Status & 0x80000000) != 0)
    {
        if (pDeviceContext->ReuseRequest) {
            pDeviceContext->ReuseRequest = NULL;
        }

        if (pDeviceContext->RequestBuffer) {
            pDeviceContext->RequestBuffer = NULL;
            pDeviceContext->ReportBuffer = NULL;
        }
    }
    RegDebug(L"PtpFilterDevicePrepareHardware end", NULL, Status);
    return Status;
}

NTSTATUS EvtDeviceReleaseHardware(WDFDEVICE Device,
    WDFCMRESLIST ResourcesTranslated)
{
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_CONTEXT pDeviceContext = GetDeviceContext(Device);

    pDeviceContext->RequestLength = 1;

    if (pDeviceContext->ReuseRequest) {
        WdfObjectDelete(pDeviceContext->ReuseRequest);

        pDeviceContext->ReuseRequest = NULL;
    }

    if (pDeviceContext->RequestBuffer) {
        WdfObjectDelete(pDeviceContext->RequestBuffer);

        pDeviceContext->RequestBuffer = NULL;
        pDeviceContext->ReportBuffer = NULL;
    }

    PVOID buffer = pDeviceContext->PreparsedData;
    if (buffer)
    {
        ExFreePoolWithTag(buffer, MOUHID_TAG);////0x50747046u=
        pDeviceContext->PreparsedData=NULL;
    }

    buffer = pDeviceContext->UsageListBuffer;
    if (buffer)
    {
        ExFreePoolWithTag(buffer, MOUHID_TAG);////0x50747046u=
        pDeviceContext->UsageListBuffer=NULL;
    }

    RegDebug(L"PtpFilterDeviceReleaseHardware", NULL, Status);

    return STATUS_SUCCESS;
}

NTSTATUS Hid_StartDevice(PDEVICE_CONTEXT pDeviceContext)
{
    NTSTATUS Status = STATUS_SUCCESS;

    WDFIOTARGET IoTarget = pDeviceContext->IoTarget;
    PHID_COLLECTION_INFORMATION NumberOfBytes = NULL;
    ULONG Size = sizeof(HID_COLLECTION_INFORMATION);
    BOOLEAN bGetCaps_x_ok = FALSE;
    BOOLEAN bGetCaps_y_ok = FALSE;

    RegDebug(L"Hid_StartDevice start", NULL, Status);

    Status = PtpFilter_WdfIoTargetFormatRequestForInternalIoctl(IOCTL_HID_GET_COLLECTION_INFORMATION, IoTarget, NULL, 0, (WDFMEMORY)NumberOfBytes, Size);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilter_WdfIoTargetFormatRequestForInternalIoctl failed", NULL, Status);
        return Status;
    }

    ULONG DescriptorSize = NumberOfBytes->DescriptorSize;
    RegDebug(L"Hid_StartDevice DescriptorSize=", NULL, DescriptorSize);

    PVOID PreparsedData = ExAllocatePoolWithTag(NonPagedPool, (size_t)DescriptorSize, MOUHID_TAG);
    pDeviceContext->PreparsedData = (PHIDP_PREPARSED_DATA)PreparsedData;

    Status = PtpFilter_WdfIoTargetFormatRequestForInternalIoctl(IOCTL_HID_GET_COLLECTION_DESCRIPTOR, IoTarget, NULL, 0, (WDFMEMORY)PreparsedData, DescriptorSize);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterCallHidClassSynchronously failed", NULL, Status);
        return Status;
    }
    
    Status = PtpFilterGetReportId((PULONG)&pDeviceContext->MultiTouchReportID, pDeviceContext);
    RegDebug(L"Hid_StartDevice PtpFilterGetReportId=", NULL, pDeviceContext->MultiTouchReportID);

    Status = HidP_GetCaps(pDeviceContext->PreparsedData, &pDeviceContext->Capabilities);
    if (Status != HIDP_STATUS_SUCCESS)//(!NT_SUCCESS(Status))
    {
        /* failed to get capabilities */
        RegDebug(L"HidP_GetCaps failed", NULL, Status);
        ExFreePoolWithTag(PreparsedData, MOUHID_TAG);
        return Status;
    }

    HIDP_CAPS Caps = pDeviceContext->Capabilities;

    /* verify capabilities */
    if (!((Caps.UsagePage == HID_USAGE_PAGE_DIGITIZER && Caps.Usage == HID_USAGE_DIGITIZER_TOUCH_PAD) \
        || (Caps.UsagePage == HID_USAGE_PAGE_GENERIC && Caps.Usage == HID_USAGE_GENERIC_MOUSE)))
    {
        /* not supported */
        RegDebug(L"not supported Capabilities", NULL, Status);
        ExFreePoolWithTag(PreparsedData, MOUHID_TAG);
        return STATUS_UNSUCCESSFUL;
    }

    ULONG UsageLength = Caps.InputReportByteLength;
    if (!UsageLength)
    {
        RegDebug(L"NumberInputButtonCaps is 0", NULL, Status);
        return STATUS_UNSUCCESSFUL;
    }

    pDeviceContext->ReportLength = UsageLength;
    RegDebug(L"Hid_StartDevice pDeviceContext->ReportLength=", NULL, pDeviceContext->ReportLength);
    PVOID Buffer = ExAllocatePoolWithTag(NonPagedPool, 2 * sizeof(USAGE) * UsageLength, MOUHID_TAG);
    pDeviceContext->UsageListBuffer = Buffer;

    Status = PtpFilterGetMaxFingers(pDeviceContext);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterGetMaxFingers failed", NULL, Status);
        return Status;
    }
    RegDebug(L"Hid_StartDevice pDeviceContext->MaxFingerCount=", NULL, pDeviceContext->MaxFingerCount);

    
    ULONG ValueCapsLength = Caps.NumberInputValueCaps;
    if (!ValueCapsLength)
    {
        RegDebug(L"NumberInputValueCaps is 0", NULL, Status);
        return STATUS_UNSUCCESSFUL;
    }
    pDeviceContext->ValueCapsLength = ValueCapsLength;
    size_t size = 72 * ValueCapsLength;
    PHIDP_VALUE_CAPS HidValueCaps = (PHIDP_VALUE_CAPS)ExAllocatePoolWithTag(NonPagedPool, size, MOUHID_TAG);
    if (!HidValueCaps)
    {
        /* no memory */
        RegDebug(L"ExAllocatePoolWithTag for value caps failed", NULL, Status);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = HidP_GetValueCaps(HidP_Input, HidValueCaps, (PUSHORT)&ValueCapsLength, pDeviceContext->PreparsedData);
    if (!NT_SUCCESS(Status) || !ValueCapsLength) {
        RegDebug(L"HidP_GetValueCaps failed", NULL, Status);
        return Status;
    }

    ULONG i = 0;
    do {
        if (!bGetCaps_x_ok && HidValueCaps[i].Range.UsageMin == HID_USAGE_GENERIC_X && HidValueCaps->UsagePage == HID_USAGE_PAGE_GENERIC) {
            double Physical_Length_X = GetPhysicalValue(HidValueCaps[i].UnitsExp, HidValueCaps[i].Units, HidValueCaps[i].PhysicalMax, HidValueCaps[i].PhysicalMin);
            pDeviceContext->TouchPad_Physical_Length_X = (float)Physical_Length_X;//触摸板x轴物理长度
            RegDebug(L"Hid_StartDevice TouchPad_Physical_Length_X=", NULL, (ULONG)pDeviceContext->TouchPad_Physical_Length_X);
            bGetCaps_x_ok = TRUE;
            pDeviceContext->TouchPad_Physical_DotDistance_X = (float)Physical_Length_X/ (HidValueCaps[i].LogicalMax - HidValueCaps[i].LogicalMin);//触摸板x轴点距DotDistance
        }

        if (!bGetCaps_y_ok && HidValueCaps[i].Range.UsageMin == HID_USAGE_GENERIC_Y && HidValueCaps->UsagePage == HID_USAGE_PAGE_GENERIC) {
            double Physical_Length_Y = GetPhysicalValue(HidValueCaps[i].UnitsExp, HidValueCaps[i].Units, HidValueCaps[i].PhysicalMax, HidValueCaps[i].PhysicalMin);
            pDeviceContext->TouchPad_Physical_Length_Y = (float)Physical_Length_Y;//触摸板x轴物理长度
            RegDebug(L"Hid_StartDevice TouchPad_Physical_Length_Y=", NULL, (ULONG)pDeviceContext->TouchPad_Physical_Length_Y);
            bGetCaps_y_ok = TRUE;
            pDeviceContext->TouchPad_Physical_DotDistance_Y = (float)Physical_Length_Y / (HidValueCaps[i].LogicalMax - HidValueCaps[i].LogicalMin);//触摸板y轴点距DotDistance
        }

        if (bGetCaps_x_ok && bGetCaps_x_ok) {
            break;
        }

        ++i;//
    } while (i != ValueCapsLength);


    ExFreePoolWithTag(HidValueCaps, MOUHID_TAG);
    RegDebug(L"Hid_StartDevice ok", NULL, Status);

    return STATUS_SUCCESS;

}

NTSTATUS PtpFilterGetMaxFingers(PDEVICE_CONTEXT pDeviceContext)
{
    NTSTATUS Status = STATUS_SUCCESS;

    USHORT HIDP_LINK_COLLECTION;
    ULONG LinkCollectionNodesLength[3];
    USHORT ValueCapsLength[2];
    HIDP_VALUE_CAPS ValueCaps;
    

    PHIDP_PREPARSED_DATA PreparsedData = pDeviceContext->PreparsedData;
    pDeviceContext->MaxFingerCount = 0;
    if (PreparsedData)
    {
        LinkCollectionNodesLength[0] = 0;
        Status = HidP_GetLinkCollectionNodes(0, LinkCollectionNodesLength, PreparsedData);
        if (Status == -1072627705)//0xc0110007
        {
            HIDP_LINK_COLLECTION = 1;
            ValueCapsLength[0] = 1;
            if (LinkCollectionNodesLength[0] > 1)
            {
                do
                {
                    Status = HidP_GetSpecificValueCaps(//status_v3
                        HidP_Input,
                        HID_USAGE_PAGE_DIGITIZER,
                        HIDP_LINK_COLLECTION,
                        0x51u,////USAGE_ID(Contact Identifier) //触摸点ID号
                        &ValueCaps,
                        ValueCapsLength,
                        pDeviceContext->PreparsedData);
                    if (Status == HIDP_STATUS_SUCCESS)
                    {
                        ++pDeviceContext->MaxFingerCount;
                    }

                    ++HIDP_LINK_COLLECTION;

                } while (HIDP_LINK_COLLECTION < LinkCollectionNodesLength[0]);
            }
            if (!pDeviceContext->MaxFingerCount)
            {
                Status = STATUS_BAD_DATA;//((NTSTATUS)0xC000090BL)
            }
        }
        else
        {
            RegDebug(L"PtpFilterGetMaxFingers HidP_GetLinkCollectionNodes failed", NULL, Status);
        }  
    }
    else
    {
        RegDebug(L"PtpFilterGetMaxFingers PreparsedData failed", NULL, Status);
    }

    RegDebug(L"PtpFilterGetMaxFingers ok", NULL, Status);
    return Status;
}


double GetPhysicalValue(ULONG UnitsExp, ULONG Units, LONG PhysicalMax, LONG PhysicalMin)
{
    double Result;//USHORT

    BYTE* pUnitExp;
    ULONG UnitSystemValue;

    USHORT Ratio;//单位制式尺成公制毫米比例 1尺=多少毫米
    USHORT ExponentValue;//指数值
    USHORT UnitValue;
    ULONG PhysicaDifference;

    char UnitExponent = UnitsExp & 0xF;//单位指数,取输入值的低4位
    Result = 0;

    UINT8 i = 0;
    pUnitExp = (BYTE*)UnitExponent_Table;//指针指向表格开头
    do
    {
        if (*pUnitExp == UnitExponent)
            break;
        ++i;
        pUnitExp += 4;//取表格内下个数据，4为单个数据长度
    } while (i < 0xB);

    ULONG UnitSystem = Units & 0xF;////表格竖列Column方向表示单位制式，行row表示单位用途种类
    if (UnitSystem < 5)
    {
        UnitSystemValue = Unit_TABLE[UnitSystem] - 1;
        if (UnitSystemValue)
        {
            if (UnitSystemValue != 2)
                return Result;
            Ratio = 2540;  //英制1英尺=2540毫米
        }
        else
        {
            Ratio = 1000; //公制1米=1000毫米
        }
        if (i < 0xB)
        {
            ExponentValue = (USHORT)UnitExponent_Table[2 * i + 1];
            UnitValue = (USHORT)ExponentTransfor(ExponentValue);
            PhysicaDifference = PhysicalMax - PhysicalMin;
            if (ExponentValue >= 0)
            {
                Result = UnitValue * PhysicaDifference * Ratio;
            }
            else if (UnitValue)
            {
                Result = PhysicaDifference * (unsigned int)Ratio / UnitValue;
            }
        }
    }
    return Result;
}

double ExponentTransfor(SHORT Value)
{
    int v1; // edx
    DOUBLE result; // rax
    bool v3; // zf

    v1 = 10;
    result = 1;
    v3 = Value == 0;
    if (Value < 0)
    {
        Value = -Value;
        v3 = Value == 0;
    }
    if (!v3)
    {
        do
        {
            if ((Value & 1) != 0)
                result = (unsigned int)(v1 * result);
            v1 *= v1;
            Value >>= 1;
        } while (Value);
    }
    return result;
}

NTSTATUS PtpFilter_WdfIoTargetFormatRequestForInternalIoctl(ULONG IoControlCode, WDFIOTARGET IoTarget, WDFMEMORY InputBuffer, ULONG InputBuffer_size, WDFMEMORY OutputBuffer, ULONG OutputBuffer_size)
{
    PWDFMEMORY_OFFSET InputBufferOffset = NULL;
    PWDFMEMORY_OFFSET OutputBufferOffset = NULL;

    if (InputBuffer) {
        InputBufferOffset->BufferOffset = (size_t)InputBuffer;
        InputBufferOffset->BufferLength = InputBuffer_size;

    }

    if (OutputBuffer) {
        memset(OutputBuffer, 0, OutputBuffer_size);
        OutputBufferOffset->BufferOffset = (size_t)OutputBuffer;
        OutputBufferOffset->BufferLength = OutputBuffer_size;

    }

    return WdfIoTargetFormatRequestForInternalIoctl(IoTarget, NULL, IoControlCode, InputBuffer, InputBufferOffset, OutputBuffer, OutputBufferOffset);
}

//设备卸载,在这里释放EvtDeviceAdd分配的资源, WDF资源不用释放，WDF框架会自动回收
static VOID EvtCleanupCallback(WDFOBJECT Object)
{
    PDEVICE_CONTEXT pDeviceContext = GetDeviceContext((WDFDEVICE)Object);

    UNREFERENCED_PARAMETER(pDeviceContext);
    RegDebug(L"EvtCleanupCallback ok", NULL, 0);
}

NTSTATUS EvtDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit)
{
    UNREFERENCED_PARAMETER(Driver);

    NTSTATUS Status;
    WDFDEVICE hDevice;

    WDF_OBJECT_ATTRIBUTES DeviceAttributes;
    WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;

    WDFQUEUE             queue;
    WDF_IO_QUEUE_CONFIG queueConfig;

    WdfFdoInitSetFilter(DeviceInit);

    //设置电源回调函数
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

    //设备处于工作（供电D0状态）或者非工作状态
    pnpPowerCallbacks.EvtDevicePrepareHardware = EvtDevicePrepareHardware;
    pnpPowerCallbacks.EvtDeviceReleaseHardware = EvtDeviceReleaseHardware;
    pnpPowerCallbacks.EvtDeviceD0Entry = EvtDeviceD0Entry;
    pnpPowerCallbacks.EvtDeviceD0Exit = EvtDeviceD0Exit;

    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks); ///

    ////创建过滤设备
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&DeviceAttributes, DEVICE_CONTEXT);
    DeviceAttributes.EvtCleanupCallback = EvtCleanupCallback; //设备删除

    Status = WdfDeviceCreate(&DeviceInit, &DeviceAttributes, &hDevice);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterCreateDevice WdfDeviceCreate failed", NULL, Status);
        return Status;
    }

    PDEVICE_CONTEXT pDeviceContext = GetDeviceContext(hDevice);

    pDeviceContext->hDevice = hDevice;
    pDeviceContext->IoTarget = WdfDeviceGetIoTarget(hDevice);

    pDeviceContext->RequestDataAvailableFlag = FALSE;
    pDeviceContext->RequestLength = 0;

    ///创建 Dispatch Queue
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);

    queueConfig.EvtIoDeviceControl = PtpFilterEvtIoDeviceControl;
    queueConfig.EvtIoInternalDeviceControl = PtpFilterEvtIoDeviceControl;
    queueConfig.EvtIoRead = PtpFilterEvtIoRead;

    Status = WdfIoQueueCreate(
        hDevice,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &queue);

    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterCreateDevice WdfIoQueueCreate failed", NULL, Status);
        return Status;
    }

    WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);
    Status = WdfIoQueueCreate(
        hDevice,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &pDeviceContext->ResetNotificationQueue);

    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterCreateDevice WdfIoQueueCreate ReportQueue_32 failed", NULL, Status);
        return Status;
    }

    WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);
    queueConfig.PowerManaged = WdfFalse;//测试代码WdfTrue WdfFalse  WdfUseDefault
    Status = WdfIoQueueCreate(
        hDevice,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &pDeviceContext->ReadReportQueue);

    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterCreateDevice WdfIoQueueCreate ReportQueue_56 failed", NULL, Status);
        return Status;
    }

   // sub_140001038(pDeviceContext);//测试什么功能

    Status = WdfSpinLockCreate(NULL, &pDeviceContext->ReadLoopSpinLock);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"WdfSpinLockCreate ReadLoopSpinLock failed", NULL, Status);
        return Status;
    }

    Status = WdfSpinLockCreate(NULL, &pDeviceContext->ProcessedBufferSpinLock);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterCreateDevice WdfSpinLockCreate ProcessedBufferSpinLock failed", NULL, Status);
        return Status;
    }

    //Status = WdfDeviceCreateDeviceInterface(
    //    hDevice,
    //    &GUID_DEVINTERFACE_PtpFilter,
    //    NULL // ReferenceString
    //);

    //if (NT_SUCCESS(Status))
    //{
    //    RegDebug(L"PtpFilterCreateDevice WdfDeviceCreateDeviceInterface failed", NULL, Status);
    //    return Status;
    //}

    //BOOLEAN IsInterfaceEnabled = TRUE;
    //WdfDeviceSetDeviceInterfaceState(
    //    hDevice,
    //    &GUID_DEVINTERFACE_PtpFilter,
    //    NULL, // ReferenceString
    //    IsInterfaceEnabled);

    RegDebug(L"PtpFilterCreateDevice PtpFilterCreateDevice ok", NULL, 0);
    return STATUS_SUCCESS;
}


NTSTATUS EvtDeviceD0Entry(WDFDEVICE Device, WDF_POWER_DEVICE_STATE PreviousState)
{

    NTSTATUS   status = STATUS_SUCCESS;
    PDEVICE_CONTEXT pDeviceContext = GetDeviceContext(Device);
    RegDebug(L"EvtDeviceD0Entry", NULL, status);


    UNREFERENCED_PARAMETER(pDeviceContext);

    UNREFERENCED_PARAMETER(PreviousState);
    /////
    return STATUS_SUCCESS;
}


NTSTATUS EvtDeviceD0Exit(WDFDEVICE Device, WDF_POWER_DEVICE_STATE TargetState)
{
    RegDebug(L"EvtDeviceD0Exit", NULL, 0);
    PDEVICE_CONTEXT pDeviceContext = GetDeviceContext(Device);

    UNREFERENCED_PARAMETER(pDeviceContext);
    UNREFERENCED_PARAMETER(TargetState);

    return STATUS_SUCCESS;
}

void PtpFilterEvtIoRead(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_ size_t Length)
{
    WDFDEVICE Device = WdfIoQueueGetDevice(Queue);
    PDEVICE_CONTEXT pDeviceContext = GetDeviceContext(Device);
    BOOLEAN bRequestForwardFlag = FALSE;
    EVT_WDF_REQUEST_COMPLETION_ROUTINE PtpFilter_EvtRequestIoctlCompletionRoutine;
    UNREFERENCED_PARAMETER(Length);

    runtimes++;
    RegDebug(L"EVT_IO_READ runtimes", NULL, runtimes);

    WdfSpinLockAcquire(pDeviceContext->ReadLoopSpinLock);

    NTSTATUS Status = PtpFilterCheckAndStartReadLoop(pDeviceContext, Request, PtpFilter_EvtRequestIoctlCompletionRoutine);
    if (NT_SUCCESS(Status)) {

        Status = PtpFilterProcessCurrentRequest(pDeviceContext, Request, &bRequestForwardFlag);
        if (!NT_SUCCESS(Status)) {
            RegDebug(L"PtpFilterEvtIoRead PtpFilterProcessCurrentRequest failed", NULL, Status);
        }
    }
    else {
        RegDebug(L"PtpFilterEvtIoRead PtpFilterCheckAndStartReadLoop failed", NULL, Status);
    }

    RegDebug(L"PtpFilterEvtIoRead PtpFilterCheckAndStartReadLoop ok", NULL, Status);

    WdfSpinLockRelease(pDeviceContext->ReadLoopSpinLock);
    RegDebug(L"PtpFilterEvtIoRead WdfSpinLockRelease ok", NULL, Status);

    if (!bRequestForwardFlag) {
        WdfRequestComplete(Request, Status);
    }

    RegDebug(L"PtpFilterEvtIoRead end", NULL, Status);
}


NTSTATUS PtpFilterGetReportId(PULONG UsageValue, PDEVICE_CONTEXT pDeviceContext)
{
    NTSTATUS Status;
    *UsageValue = 0;

    Status = HidP_GetUsageValue(
        HidP_Input,
        HID_USAGE_PAGE_DIGITIZER,
        0,//HIDP_LINK_COLLECTION//测试一下
        0x05u,//USAGE ID(REPORT_ID (Touch pad)) //  REPORTID_MULTITOUCH
        UsageValue,//返回值Contact Identifier
        pDeviceContext->PreparsedData,
        (PCHAR)pDeviceContext->ReportBuffer,
        NULL);

    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterGetReportId HidP_GetUsageValue failed", NULL, Status);
    }

    RegDebug(L"PtpFilterGetReportId ok", NULL, Status);
    return Status;
}


NTSTATUS PtpFilterReadContactId(PULONG UsageValue, USHORT HIDP_LINK_COLLECTION, CHAR* Report, PDEVICE_CONTEXT pDeviceContext)
{
    NTSTATUS Status;
    *UsageValue = 0;

    Status = HidP_GetUsageValue(
        HidP_Input,
        HID_USAGE_PAGE_DIGITIZER,
        HIDP_LINK_COLLECTION,
        0x51u,//USAGE ID(Contact Identifier) //触摸点ID号
        UsageValue,//返回值Contact Identifier
        pDeviceContext->PreparsedData,
        Report,
        pDeviceContext->ReportLength);

    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterReadContactId HidP_GetUsageValue failed", NULL, Status);
    }

    RegDebug(L"PtpFilterReadContactId ok", NULL, Status);
    return Status;
}

NTSTATUS PtpFilterReadContactTX(PULONG UsageValue, USHORT HIDP_LINK_COLLECTION, CHAR* Report, PDEVICE_CONTEXT pDeviceContext)//未被调用？？
{
    NTSTATUS Status;

    *UsageValue = 0;
    Status = HidP_GetUsageValue(
        HidP_Input,
        HID_USAGE_PAGE_GENERIC,
        HIDP_LINK_COLLECTION,
        HID_USAGE_GENERIC_X,//USAGE(X)       X移动
        UsageValue,////获得X移动返回值
        pDeviceContext->PreparsedData,
        Report,
        pDeviceContext->ReportLength);

    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterReadContactTX HidP_GetUsageValue HID_USAGE_GENERIC_X failed", NULL, Status);
    }

    RegDebug(L"PtpFilterReadContactTX ok", NULL, Status);
    return Status;
}

NTSTATUS PtpFilterReadContactTY(PULONG UsageValue, USHORT HIDP_LINK_COLLECTION, CHAR *Report, PDEVICE_CONTEXT pDeviceContext)//未被调用？？
{
    NTSTATUS Status;
    *UsageValue = 0;

    Status = HidP_GetUsageValue(
        HidP_Input,
        HID_USAGE_PAGE_GENERIC,
        HIDP_LINK_COLLECTION,
        HID_USAGE_GENERIC_Y,//USAGE(Y)       Y移动
        UsageValue,//获得Y移动返回值
        pDeviceContext->PreparsedData,
        Report,
        pDeviceContext->ReportLength);

    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterReadContactTY HidP_GetUsageValue HID_USAGE_GENERIC_Y failed", NULL, Status);
    }

    RegDebug(L"PtpFilterReadContactTY ok", NULL, Status);
    return Status;
}


NTSTATUS PtpFilterSetUsageValuePosX(ULONG UsageValue, USHORT HIDP_LINK_COLLECTION, CHAR *Report, PDEVICE_CONTEXT pDeviceContext)
{
    return HidP_SetUsageValue(
        HidP_Input,
        HID_USAGE_PAGE_GENERIC,
        HIDP_LINK_COLLECTION,
        HID_USAGE_GENERIC_X,
        UsageValue,
        pDeviceContext->PreparsedData,
        Report,
        pDeviceContext->ReportLength);
}


NTSTATUS PtpFilterSetUsageValuePosY(ULONG UsageValue, USHORT HIDP_LINK_COLLECTION, CHAR *Report, PDEVICE_CONTEXT pDeviceContext)
{
    return HidP_SetUsageValue(
        HidP_Input,
        HID_USAGE_PAGE_GENERIC,
        HIDP_LINK_COLLECTION,
        HID_USAGE_GENERIC_Y,
        UsageValue,
        pDeviceContext->PreparsedData,
        Report,
        pDeviceContext->ReportLength);
}


NTSTATUS PtpFilterCheckAndStartReadLoop(PDEVICE_CONTEXT pDeviceContext, WDFREQUEST Request, EVT_WDF_REQUEST_COMPLETION_ROUTINE PtpFilter_EvtRequestIoctlCompletionRoutine)
{
    NTSTATUS Status = STATUS_SUCCESS;
    if (!pDeviceContext->RequestDataAvailableFlag && !pDeviceContext->RequestLength)
    {
        pDeviceContext->RequestDataAvailableFlag = TRUE; 

        RegDebug(L"Read Loop Started", NULL, 0);

        PIRP pIrp = WdfRequestWdmGetIrp(Request);

        PHID_XFER_PACKET pHidPacket = (PHID_XFER_PACKET)pIrp->UserBuffer;

        pDeviceContext->ReportBuffer = (PCHAR)pHidPacket->reportBuffer;

        Status = PtpFilterSendReadRequest(pDeviceContext, PtpFilter_EvtRequestIoctlCompletionRoutine);
        if (!NT_SUCCESS(Status)) {
            RegDebug(L"PtpFilterCheckAndStartReadLoop PtpFilterSendReadRequest failed", NULL, Status);
            pDeviceContext->RequestDataAvailableFlag = FALSE;
        }
    }

    return Status;
}


NTSTATUS PtpFilterSendReadRequest(PDEVICE_CONTEXT pDeviceContext, EVT_WDF_REQUEST_COMPLETION_ROUTINE PtpFilter_EvtRequestIoctlCompletionRoutine)
{
    WDFREQUEST Request = pDeviceContext->ReuseRequest;

    /////重新初始化
    WDF_REQUEST_REUSE_PARAMS reuseParams;

    WDF_REQUEST_REUSE_PARAMS_INIT(
        &reuseParams,
        WDF_REQUEST_REUSE_NO_FLAGS,
        STATUS_SUCCESS
    );

    NTSTATUS Status = WdfRequestReuse(Request, &reuseParams);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterSendReadRequest WdfRequestReuse failed", NULL, Status);
        return Status;
    }

    //测试pDeviceContext->OutputBuffer = pDeviceContext->RequestBuffer;
    Status = WdfIoTargetFormatRequestForRead(pDeviceContext->IoTarget, Request, pDeviceContext->OutputBuffer, 0, 0);//pDeviceContext->OutputBuffer未初始化？并且pDeviceContext->RequestBuffer也未被调用处理，
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterSendReadRequest WdfIoTargetFormatRequestForRead failed", NULL, Status);
        return Status;
    }

    WdfRequestSetCompletionRoutine(Request, PtpFilter_EvtRequestIoctlCompletionRoutine, pDeviceContext);

    PIRP pIrp = WdfRequestWdmGetIrp(Request);

    PHID_XFER_PACKET pHidPacket = (PHID_XFER_PACKET)pIrp->UserBuffer;

    pDeviceContext->ReportBuffer = (PCHAR)pHidPacket->reportBuffer;

    BOOLEAN ret = WdfRequestSend(Request, pDeviceContext->IoTarget, NULL);
    if (!ret) {
        Status = WdfRequestGetStatus(pDeviceContext->ReuseRequest);
        RegDebug(L"PtpFilterSendReadRequest WdfRequestSend failed", NULL, Status);
        return Status;
    }

    RegDebug(L"PtpFilterSendReadRequest ok", NULL, Status);
    return Status;
}


void PtpFilter_EvtRequestIoctlCompletionRoutine(WDFREQUEST Request, WDFIOTARGET Target, PWDF_REQUEST_COMPLETION_PARAMS Params, WDFCONTEXT Context)
{
    UNREFERENCED_PARAMETER(Params);
    RegDebug(L"PtpFilter_EvtRequestIoctlCompletionRoutine Params->IoStatus.Status", NULL, Params->IoStatus.Status);
    UNREFERENCED_PARAMETER(Target);

    PDEVICE_CONTEXT pDeviceContext =(PDEVICE_CONTEXT) Context;
    BOOLEAN bRequestStopFlag = TRUE;

    NTSTATUS Status = WdfRequestGetStatus(Request);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilter_EvtRequestIoctlCompletionRoutine Read request failed", NULL, Status);
        PtpFilterProcessReadReport(pDeviceContext, Status, &bRequestStopFlag);//
        if (!bRequestStopFlag) {
            return;
        }
    }

    Status = PtpFilterHandleDeviceData(pDeviceContext);//此处获取输入报告数据并处理
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilter_EvtRequestIoctlCompletionRoutine PtpFilterHandleDeviceData failed", NULL, Status);
    }

    Status = PtpFilterSendReadRequest(pDeviceContext, PtpFilter_EvtRequestIoctlCompletionRoutine);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilter_EvtRequestIoctlCompletionRoutine PtpFilterSendReadRequest failed", NULL, Status);
        pDeviceContext->RequestDataAvailableFlag = FALSE;
    }

    RegDebug(L"PtpFilter_EvtRequestIoctlCompletionRoutine ok", NULL, Status);
    RegDebug(L"PtpFilter_EvtRequestIoctlCompletionRoutine PtpFilterOnDeviceDataAvailable", NULL, Status);
}


NTSTATUS PtpFilterHandleDeviceData(PDEVICE_CONTEXT pDeviceContext)//处理设备数据
{
    NTSTATUS Status = STATUS_SUCCESS;
    WDFSPINLOCK ProcessedBufferSpinLock = pDeviceContext->ProcessedBufferSpinLock;

    ULONG CurrentFingersCountValue = 0;

    BOOLEAN boolFlag_FingerNotChanged =FALSE;

    WdfSpinLockAcquire(ProcessedBufferSpinLock);

    Status = PtpFilterGetFingersCount(pDeviceContext, &CurrentFingersCountValue, &boolFlag_FingerNotChanged);
    if (!NT_SUCCESS(Status)) {
    
        //此处已经获取到了手指输入报告后进行数据保存
        Status =PtpFilterBufferStoreReport(pDeviceContext->REPORT_BUFFER_SIZE, (PCHAR)pDeviceContext->ReportBuffer,pDeviceContext->ReportLength, (UCHAR)CurrentFingersCountValue);
        if (NT_SUCCESS(Status)) {
            RegDebug(L"PtpFilterHandleDeviceData PtpFilterBufferProcessData rawDataProcessed No fingers in report", NULL, Status);

            PTP_REPORT* pTPreport = (PTP_REPORT*)pDeviceContext->LastRequestReportBuffer;
            RtlZeroMemory(pTPreport, pDeviceContext->ReportLength);
            pTPreport->ReportID = pDeviceContext->MultiTouchReportID;
            pTPreport->ScanTime = 0;
            RtlCopyMemory(pDeviceContext->CurrentRequestReportBuffer, pDeviceContext->LastRequestReportBuffer, pDeviceContext->ReportLength);//

            RegDebug(L"PtpFilterHandleDeviceData PtpFilterGetFingersCount failed", NULL, Status);

            Status =PtpFilterProcessPendingRequest(pDeviceContext);
            if (NT_SUCCESS(Status)) {
                goto Exit;
            }

            RegDebug(L"PtpFilterHandleDeviceData PtpFilterProcessPendingRequest failed", NULL, Status);
            goto Exit;
        }

        RegDebug(L"PtpFilterHandleDeviceData PtpFilterBufferStoreReport failed", NULL, Status);
        goto Exit;
    }

    Status = PtpFilterBufferStoreReport(pDeviceContext->REPORT_BUFFER_SIZE, (PCHAR)pDeviceContext->ReportBuffer, pDeviceContext->ReportLength, (UCHAR)CurrentFingersCountValue);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterHandleDeviceData PtpFilterBufferStoreReport failed", NULL, Status);
        goto Exit;
    }
    if (boolFlag_FingerNotChanged)
    {
        //Status = PtpFilterProcessInputFrame(pDeviceContext);
        if (!NT_SUCCESS(Status)) {
            RegDebug(L"PtpFilterHandleDeviceData PtpFilterProcessInputFrame failed", NULL, Status);
            //pDeviceContext->LastFingerCount = 0;
            //pDeviceContext->CurrentFingerCount = 0;
        }
    }
Exit:
    RegDebug(L"PtpFilterHandleDeviceData ok", NULL, Status);
    WdfSpinLockRelease (pDeviceContext->ProcessedBufferSpinLock);
    RegDebug(L"PtpFilterHandleDeviceData end", NULL, Status);
    return Status;

}

NTSTATUS PtpFilterProcessPendingRequest(PDEVICE_CONTEXT pDeviceContext)
{    
    WDFREQUEST OutRequest;

    NTSTATUS Status = WdfIoQueueRetrieveNextRequest(pDeviceContext->ReadReportQueue, &OutRequest);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterProcessPendingRequest WdfIoQueueRetrieveNextRequest err", NULL, Status);
        return Status;
    }

    PTP_REPORT* pTPreport = (PTP_REPORT*)pDeviceContext->CurrentRequestReportBuffer;
    Status = PtpFilterCompleteReadRequest(OutRequest, pTPreport, pDeviceContext->ReportLength);
    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterProcessPendingRequest PtpFilterCompleteReadRequest failed", NULL, Status);
        return Status;
    }

    RegDebug(L"PtpFilterProcessPendingRequest ok", NULL, Status);
    return Status;
}


NTSTATUS PtpFilterBufferStoreReport(USHORT REPORT_BUFFER_SIZE, PCHAR ReportBuffer, ULONG ReportLength, UCHAR CurrentFingersCountValue)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PTP_REPORT*  Struct_tp_packet = (PTP_REPORT * )ExAllocatePoolWithTag(NonPagedPool, sizeof(PTP_REPORT), MOUHID_TAG);//30 length手指数据长度

    if (Struct_tp_packet) {

        PVOID Struct_tp_finger_data = Struct_tp_packet->Contacts; //REPORT_BUFFER_SIZE = 手指个数 * sizeof(PTP_CONTACT)

        if (Struct_tp_finger_data) {
            memset(Struct_tp_finger_data, 0, REPORT_BUFFER_SIZE);

            if (REPORT_BUFFER_SIZE >= ReportLength) {
                memmove(Struct_tp_packet->Contacts, ReportBuffer, ReportLength);
            }

            Struct_tp_packet->ContactCount = CurrentFingersCountValue;

        }
        else
        {
            RegDebug(L"PtpFilterBufferStoreReport ExAllocatePoolWithTag failed", NULL, Status);

            Status = STATUS_MEMORY_NOT_ALLOCATED; //     ((NTSTATUS)0xC00000A0L)
            ExFreePoolWithTag(Struct_tp_packet, MOUHID_TAG);///0x50747046u=MOUHID_TAG
        }
    }
    else
    {
        RegDebug(L"PtpFilterBufferStoreReport ExAllocatePoolWithTag for Struct_tp_packet failed", NULL, Status);
        Status = STATUS_MEMORY_NOT_ALLOCATED;//     ((NTSTATUS)0xC00000A0L)
    }

    RegDebug(L"PtpFilterBufferStoreReport ok", NULL, Status);
    return Status;
}

void PtpFilterProcessReadReport(PDEVICE_CONTEXT pDeviceContext, NTSTATUS Status_In, BOOLEAN *bRequestStopFlag)
{

    WDFREQUEST OutRequest;

    if (pDeviceContext->RequestLength)//
    {
       pDeviceContext->RequestDataAvailableFlag = FALSE;
        *bRequestStopFlag = FALSE;////Pending_a3=FALSE;??
    }
    else
    {
        WdfSpinLockAcquire(pDeviceContext->ReadLoopSpinLock);
        NTSTATUS Status = WdfIoQueueRetrieveNextRequest(pDeviceContext->ReadReportQueue, &OutRequest);
        if (NT_SUCCESS(Status)) {

            *bRequestStopFlag = TRUE;//Pending_a3=TRUE;??
            WdfSpinLockRelease (pDeviceContext->ReadLoopSpinLock);
            WdfRequestComplete(OutRequest, Status_In);//??
        }
        else
        {
            pDeviceContext->RequestDataAvailableFlag = FALSE;
            *bRequestStopFlag = FALSE;////Pending_a3=FALSE;??
            WdfSpinLockRelease (pDeviceContext->ReadLoopSpinLock);
        }
    }

    RegDebug(L"PtpFilterProcessReadReport ok", NULL, 0);
}

NTSTATUS PtpFilterGetFingersCount(PDEVICE_CONTEXT pDeviceContext, ULONG *CurrentFingersCountValue, BOOLEAN *boolFlag_FingerNotChanged)
{
    NTSTATUS Status;

    *boolFlag_FingerNotChanged = FALSE;
    Status = HidP_GetUsageValue(
        HidP_Input,
        HID_USAGE_PAGE_DIGITIZER,
        HIDP_LINK_COLLECTION_UNSPECIFIED,
        0x54u,//USAGE ID (Contact count)  =54 //Contact Count	Total number of contacts to be reported in a given report.
        CurrentFingersCountValue,
        pDeviceContext->PreparsedData,
        (PCHAR)pDeviceContext->ReportBuffer,
        pDeviceContext->ReportLength);

    if (!NT_SUCCESS(Status)) {
        RegDebug(L"PtpFilterGetFingersCount HidP_GetUsageValue failed", NULL, Status);
        return Status;
    }

    if (pDeviceContext->LastFingerCount){
        if (*CurrentFingersCountValue)
        {
            RegDebug(L"PtpFilterGetFingersCount Expected report with 0 actual count in OEM HID report", NULL, Status);
            return STATUS_BAD_DATA;//((NTSTATUS)0xC000090BL)
        }
    }
    else{
        if (!*CurrentFingersCountValue){
            RegDebug(L"PtpFilterGetFingersCount Non-zero actual count expected in OEM HID report", NULL, Status);
            return STATUS_BAD_DATA;//((NTSTATUS)0xC000090BL)
        }
        
        pDeviceContext->CurrentFingerCount = (UCHAR)(*CurrentFingersCountValue);
    }
    
    UCHAR FingerChangeCount = pDeviceContext->CurrentFingerCount- pDeviceContext->LastFingerCount;
    if (pDeviceContext->MaxFingerCount < FingerChangeCount) {
        FingerChangeCount = pDeviceContext->MaxFingerCount;
    }
    
    *CurrentFingersCountValue = FingerChangeCount;
    pDeviceContext->LastFingerCount += FingerChangeCount ;
    if (pDeviceContext->LastFingerCount == pDeviceContext->CurrentFingerCount) {
        *boolFlag_FingerNotChanged = TRUE;
    }
        
    RegDebug(L"PtpFilterGetFingersCount ok", NULL, Status);
    return Status;
}


void PtpFilterEvtIoDeviceControl(
    IN WDFQUEUE     Queue,
    IN WDFREQUEST   Request,
    IN size_t       OutputBufferLength,
    IN size_t       InputBufferLength,
    IN ULONG        IoControlCode)
{
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    NTSTATUS Status = STATUS_SUCCESS;
    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    PDEVICE_CONTEXT pDeviceContext = GetDeviceContext(device);

    UNREFERENCED_PARAMETER(pDeviceContext);
    UNREFERENCED_PARAMETER(IoControlCode);
    UNREFERENCED_PARAMETER(Status);
    Filter_DispatchPassThrough(Request, WdfDeviceGetIoTarget(device));
    return;

    //switch (IoControlCode)
    //{
    //case IOCTL_HID_GET_COLLECTION_INFORMATION://经过测试会执行
    //    RegDebug(L"IOCTL_HID_GET_COLLECTION_INFORMATION", NULL, IoControlCode);
    //    break;

    //case IOCTL_HID_GET_COLLECTION_DESCRIPTOR://经过测试会执行
    //    RegDebug(L"IOCTL_HID_GET_COLLECTION_DESCRIPTOR", NULL, IoControlCode);
    //    break;

    //case IOCTL_HID_GET_PRODUCT_STRING://经过测试会执行
    //    RegDebug(L"IOCTL_HID_GET_PRODUCT_STRING", NULL, IoControlCode);
    //    break;

    //case IOCTL_HID_GET_FEATURE://经过测试会执行
    //    RegDebug(L"IOCTL_HID_GET_FEATURE", NULL, IoControlCode);
    //    break;

    // default:
    //     RegDebug(L"STATUS_NOT_SUPPORTED", NULL, IoControlCode);
    //     Status = STATUS_NOT_SUPPORTED;
    // }

    ///////complete irp
    //WdfRequestComplete(Request, Status);

   /* if (!IoControlCode_v13) {
        
        Status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, (PVOID *)&Outresult_buffer_v26, &Outresult_bufferSize_v25);
        if (Status >= 0 && Outresult_bufferSize_v25 == 20) {
            v19 = Outresult_buffer_v26[1].m128i_i32[0];
            v20 = _mm_cvtsi128_si32(*Outresult_buffer_v26);
            v27 = *Outresult_buffer_v26;
            *(_BYTE*)v12 = v20;
            if (v20)
            {
                *(_QWORD*)(v12 + 80) = *(__int64*)((char*)v27.m128i_i64 + 4);
                *(_DWORD*)(v12 + 88) = v27.m128i_i32[3] * v27.m128i_i32[3];
                *(_DWORD*)(v12 + 92) = v19 * v19;
            }
            v21 = v27.m128i_i32[0];
            LODWORD(v23) = v27.m128i_i32[0];
            DbgPrint("START_STOP_FILTERING PARAMS, state: %d", v21);
            DbgPrint("\n");
        }
    }*/
}


VOID
Filter_DispatchPassThrough(
    _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target
)
/*++
Routine Description:

    Passes a request on to the lower driver.


--*/
{
    //
    // Pass the IRP to the target
    //
    //RegDebug(L"Filter_DispatchPassThrough", NULL, 0);

    WDF_REQUEST_SEND_OPTIONS options;
    BOOLEAN ret;
    NTSTATUS status = STATUS_SUCCESS;
    WDFDEVICE device = WdfIoQueueGetDevice(WdfRequestGetIoQueue(Request));
    PDEVICE_CONTEXT pDeviceContext = GetDeviceContext(device);

    UNREFERENCED_PARAMETER(pDeviceContext);
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

    RegDebug(L"Filter_DispatchPassThrough WdfRequestSend ok", 0, runtimes);

    return;
}

NTSTATUS  PtpFilterProcessCurrentRequest(PDEVICE_CONTEXT pDeviceContext, WDFREQUEST Request, BOOLEAN *bRequestForwardFlag)
{
    NTSTATUS Status = STATUS_SUCCESS;
    *bRequestForwardFlag = FALSE;

    WdfSpinLockAcquire(pDeviceContext->ProcessedBufferSpinLock);

    PTP_REPORT* pTPreport = (PTP_REPORT*)pDeviceContext->CurrentRequestReportBuffer;
    if (pTPreport->ReportID == pDeviceContext->MultiTouchReportID)
    {
        WdfSpinLockRelease (pDeviceContext->ProcessedBufferSpinLock);

        Status = PtpFilterCompleteReadRequest(Request, pTPreport, pDeviceContext->ReportLength);
        if (!NT_SUCCESS(Status))
        {
            RegDebug(L"PtpFilterProcessCurrentRequest PtpFilterCompleteReadRequest failed", NULL, Status);
            return Status;
        }

        *bRequestForwardFlag = TRUE;
        return Status;
    }
    
    WdfSpinLockRelease (pDeviceContext->ProcessedBufferSpinLock);

    if (!*bRequestForwardFlag)
    {
        Status = WdfRequestForwardToIoQueue(Request, pDeviceContext->ReadReportQueue);
        if (!NT_SUCCESS(Status))
        {
            RegDebug(L"PtpFilterProcessCurrentRequest WdfRequestForwardToIoQueue failed", NULL, Status);
            return Status;
        }

        *bRequestForwardFlag = TRUE;
    }

    RegDebug(L"PtpFilterProcessCurrentRequest ok", NULL, Status);
    return Status;
}

NTSTATUS PtpFilterCompleteReadRequest(WDFREQUEST Request, PVOID SourceBuffer, ULONG_PTR Information_size)
{
    PVOID OutBuffer;
    size_t Size;

    NTSTATUS Status = WdfRequestRetrieveOutputBuffer(Request, Information_size, &OutBuffer, &Size);
    if (NT_SUCCESS(Status))
    {
        memset(OutBuffer, 0, Size);
        if (Size >= Information_size) {
            memmove(OutBuffer, SourceBuffer, Information_size);
        }
            
        WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, Information_size);
    }
    else
    {
        RegDebug(L"PtpFilterCompleteReadRequest WdfRequestRetrieveOutputBuffer failed", NULL, Status);
    }

    return Status;

}

VOID RegDebug(WCHAR* strValueName, PVOID dataValue, ULONG datasizeValue)//RegDebug(L"Run debug here",pBuffer,pBufferSize);//RegDebug(L"Run debug here",NULL,0x12345678);
{
    //初始化注册表项
    UNICODE_STRING stringKey;
    RtlInitUnicodeString(&stringKey, L"\\Registry\\Machine\\Software\\RegDebug");

    //初始化OBJECT_ATTRIBUTES结构
    OBJECT_ATTRIBUTES  ObjectAttributes;
    InitializeObjectAttributes(&ObjectAttributes, &stringKey, OBJ_CASE_INSENSITIVE, NULL, NULL);

    //创建注册表项
    HANDLE hKey;
    ULONG Des;
    NTSTATUS status = ZwCreateKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, &Des);
    if (NT_SUCCESS(status))
    {
        if (Des == REG_CREATED_NEW_KEY)
        {
            KdPrint(("新建注册表项！\n"));
        }
        else
        {
            KdPrint(("要创建的注册表项已经存在！\n"));
        }
    }
    else {
        return;
    }

    //初始化valueName
    UNICODE_STRING valueName;
    RtlInitUnicodeString(&valueName, strValueName);

    if (dataValue == NULL) {
        //设置REG_DWORD键值
        status = ZwSetValueKey(hKey, &valueName, 0, REG_DWORD, &datasizeValue, 4);
        if (!NT_SUCCESS(status))
        {
            KdPrint(("设置REG_DWORD键值失败！\n"));
        }
    }
    else {
        //设置REG_BINARY键值
        status = ZwSetValueKey(hKey, &valueName, 0, REG_BINARY, dataValue, datasizeValue);
        if (!NT_SUCCESS(status))
        {
            KdPrint(("设置REG_BINARY键值失败！\n"));
        }
    }
    ZwFlushKey(hKey);
    //关闭注册表句柄
    ZwClose(hKey);
}
