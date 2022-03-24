#pragma once

#pragma once

#include <ntddk.h>
#include <ntstrsafe.h>
#include <ntintsafe.h>
#include <wdf.h>
#include <wdm.h>

#include "hidsdi.h"
#include "hidpi.h" 
#include <initguid.h>
#include <devpkey.h>


#include <hidport.h>

//
#include "spb.h"

//
#define RESHUB_USE_HELPER_ROUTINES
#include "reshub.h"

//
#include <acpiioct.h>

//
typedef BYTE* PBYTE;

#define countof(x) (sizeof(x) / sizeof(x[0]))

#define WdfDeleteObjectSafe(o)  \
    if (o != NULL) { WdfObjectDelete(o); }

//
typedef struct _ACPI_DEVICE_SETTINGS
{
    // ACPI "_DSM" HID Descriptor Address
    USHORT  HidDescriptorAddress;

} ACPI_DEVICE_SETTINGS, * PACPI_DEVICE_SETTINGS;

#include "pshpack1.h"

#define MAXFINGER_CNT 5

#define STABLE_INTERVAL_FingerSeparated_MSEC   20   // 手指分开按到触摸板的稳定时间间隔
#define STABLE_INTERVAL_FingerClosed_MSEC      100   // 手指并拢按到触摸板的稳定时间间隔 

#define MouseReport_INTERVAL_MSEC         8   // 鼠标报告间隔时间ms，以频率125hz为基准
#define ButtonPointer_Interval_MSEC      200   // 鼠标左中右键与指针操作间隔时间ms，

#define Jitter_Offset         10    // 修正触摸点轻微抖动的位移阈值


#pragma pack(push)
#pragma pack(1)
typedef struct _PTP_CONTACT {
    UCHAR		Confidence : 1;
    UCHAR		TipSwitch : 1;
    UCHAR		ContactID : 3;
    UCHAR		Padding : 3;
    USHORT		X;
    USHORT		Y;
} PTP_CONTACT, * PPTP_CONTACT;
#pragma pack(pop)

typedef struct _PTP_REPORT {
    UCHAR       ReportID;
    PTP_CONTACT Contacts[5];
    USHORT      ScanTime;
    UCHAR       ContactCount;
    UCHAR       IsButtonClicked;
} PTP_REPORT, * PPTP_REPORT;

CHAR UnitExponent_Table[16] ={0,1,2,3,4,5,6,7,-8,-7,-6,-5,-4,-3,-2,1};

typedef struct _PTP_PARSER {

    //保存追踪的手指数据
    PTP_REPORT lastfinger;
    PTP_REPORT currentfinger;

    char Mouse_Pointer_CurrentIndexNUM; //定义当前鼠标指针触摸点坐标的数据索引号，-1为未定义
    char Mouse_LButton_CurrentIndexNUM; //定义当前鼠标左键触摸点坐标的数据索引号，-1为未定义
    char Mouse_RButton_CurrentIndexNUM; //定义当前鼠标右键触摸点坐标的数据索引号，-1为未定义
    char Mouse_MButton_CurrentIndexNUM; //定义当前鼠标中键触摸点坐标的数据索引号，-1为未定义
    char Mouse_Wheel_CurrentIndexNUM; //定义当前鼠标滚轮辅助参考手指触摸点坐标的数据索引号，-1为未定义

    char Mouse_Pointer_LastIndexNUM; //定义上次鼠标指针触摸点坐标的数据索引号，-1为未定义
    char Mouse_LButton_LastIndexNUM; //定义上次鼠标左键触摸点坐标的数据索引号，-1为未定义
    char Mouse_RButton_LastIndexNUM; //定义上次鼠标右键触摸点坐标的数据索引号，-1为未定义
    char Mouse_MButton_LastIndexNUM; //定义上次鼠标中键触摸点坐标的数据索引号，-1为未定义
    char Mouse_Wheel_LastIndexNUM; //定义上次鼠标滚轮辅助参考手指触摸点坐标的数据索引号，-1为未定义

    BOOLEAN Mouse_Wheel_mode; //定义鼠标滚轮状态，0为滚轮未激活，1为滚轮激活
    BOOLEAN Mouse_Gesture3Finger_mode; //定义鼠标3指手势状态，0为3指手势未激活，1为3指手势激活
    BOOLEAN Mouse_Gesture4Finger_mode; //定义鼠标4指手势状态，0为4指手势未激活，1为4指手势激活

    LARGE_INTEGER MousePointer_DefineTime;//鼠标指针定义时间，用于计算按键间隔时间来区分判定鼠标中间和滚轮操作
    float TouchPad_ReportInterval;//定义触摸板报告间隔时间

    LARGE_INTEGER JitterFixStartTime; // 修正触摸点抖动修正时间计时器

    BYTE Scroll_IntervalCount; //定义鼠标滚动间隔计数

    ULONG tick_count;
    LARGE_INTEGER last_ticktime; //上次报告计时
    LARGE_INTEGER current_ticktime;//当前报告计时
    LARGE_INTEGER ticktime_Interval;//报告间隔时间

    //
    ULONG physicalMax_X;
    ULONG physicalMax_Y;
    ULONG logicalMax_X;
    ULONG logicalMax_Y;
    CHAR unitExp;
    UCHAR unit;

    DOUBLE physical_Width_mm;
    DOUBLE  physical_Height_mm;

    //定义手指头尺寸大小
    float thumb_width;//手指头宽度
    float thumb_height;//手指头高度
    float thumb_scale;//手指头尺寸缩放比例
    float FingerMinDistance;//定义有效的相邻手指最小距离
    float FingerClosedThresholdDistance;//定义相邻手指合拢时的最小距离
    float FingerMaxDistance;//定义有效的相邻手指最大距离
    float TouchPad_DPMM_x;//定义触摸板分辨率
    float TouchPad_DPMM_y;//定义触摸板分辨率
    float PointerSensitivity_x;//定义指针灵敏度即指针点移动量缩放比例
    float PointerSensitivity_y;//定义指针灵敏度即指针点移动量缩放比例

} PTP_PARSER, * PPTP_PARSER;

//
typedef struct _HID_DEVICE_SETTINGS
{
    // Length of the HID Descriptor
    USHORT HidDescriptorLength;

    // BCD Version
    USHORT  BcdVersion;

    // Length of the Report Descriptor
    USHORT  ReportDescriptorLength;

    // Report Descriptor Register address
    USHORT  ReportDescriptorAddress;

    // Input Register Address
    USHORT  InputRegisterAddress;

    // Input Register Maximum Length
    USHORT  InputReportMaxLength;

    // Output Register Address
    USHORT  OutputRegisterAddress;

    // Output Register Maximum Length
    USHORT  OutputReportMaxLength;

    // Command Register Address
    USHORT  CommandRegisterAddress;

    // Data Register Address
    USHORT  DataRegisterAddress;


    // I2C device metadata
    // Vendor Id
    USHORT VendorId;//HidVendorID;

    // Device Id
    USHORT ProductId;//HidProductID;

    //Revision Id
    USHORT VersionId;//HidVersionNumber;

    // Reserved Bits
    ULONG  Reserved;

} HID_DEVICE_SETTINGS, * PHID_DEVICE_SETTINGS;

#include "poppack.h"

//
// Registry settings
//
typedef struct _REGISTRY_DEVICE_SETTINGS
{
    BOOLEAN CompactWriteWriteSequences;

} REGISTRY_DEVICE_SETTINGS, * PREGISTRY_DEVICE_SETTINGS;

//
// Device context
typedef struct _DEVICE_CONTEXT
{
    // Handle to the WDF device.
    WDFDEVICE                   FxDevice;//I2C_Device;

    // Locks
    WDFSPINLOCK                 InProgressLock;

    // Interrupts, DPCs and workitems
    WDFINTERRUPT                ReadInterrupt;
    volatile BOOLEAN            ProcessInterrupts;
    BOOLEAN                     DpcRerun;
    BOOLEAN                     DpcInProgess;

    // Queues 
    WDFQUEUE                    IoctlQueue;
    WDFQUEUE                    ReportQueue;
    WDFQUEUE                    CompletionQueue;
    WDFQUEUE                    IdleQueue;
    WDFQUEUE	                ResetNotificationQueue;

    // SPB specific settings
    WDFIOTARGET                 SpbIoTarget;
    LARGE_INTEGER               SpbConnectionId;
    WDFREQUEST                  SpbRequest;

    // ACPI specific settings
    ACPI_DEVICE_SETTINGS        AcpiSettings;

    // HID specific settings
    HID_DEVICE_SETTINGS         HidSettings;
    PBYTE                       pHidInputReport;
    LIST_ENTRY                  InputReportListHead;
    WDFSPINLOCK                 InputReportListLock;
    ULONG                       InputReportListCount;

    // Flags
    BOOLEAN                     HostInitiatedResetActive;

    // Device Reset Notification members
    WDFSPINLOCK                 DeviceResetNotificationSpinLock;
    WDFREQUEST                  DeviceResetNotificationRequest;


    // Timer
    WDFTIMER                   timerHandle;  //用于控制请求

    // Windows PTP context
    BOOLEAN PtpInputOn;
    BOOLEAN PtpReportTouch;
    BOOLEAN PtpReportButton;

    BOOLEAN  HidReportDescriptorSaved;
    BOOLEAN  SetFeatureReady;
    BOOLEAN  MouseConnected;
    PDEVICE_OBJECT MouseFilterDeviceObject;
    WDFREQUEST ReuseSend2MouseRequest;
    WDFMEMORY     RequestBuffer;  // 请求的Buffer
    PIRP     ReuseSend2MouseIrp;

    PTP_PARSER  tp_settings;  //PTP_PARSER数据
    UCHAR inputModeReportSize;
    UCHAR inputModeReportId;
    UCHAR funswitchReportSize;
    UCHAR funswitchReportId;

    PBYTE pReportDesciptorData;  

} DEVICE_CONTEXT, * PDEVICE_CONTEXT;


//
// Request context
typedef struct _REQUEST_CONTEXT
{
    // Handle to the WDF device
    WDFDEVICE FxDevice;

    // Handle to the WDFMemory object
    WDFMEMORY FxMemory;

} REQUEST_CONTEXT, * PREQUEST_CONTEXT;


//
// Memory context
typedef struct _MEMORY_CONTEXT
{
    // linked list entry that holds an input report
    LIST_ENTRY InputReportListEntry;

} MEMORY_CONTEXT, * PMEMORY_CONTEXT;


//
// Workitem context
typedef struct _WORKITEM_CONTEXT
{
    // Handle to a WDF device object
    WDFDEVICE FxDevice;

    // Handle to a WDF request object
    WDFREQUEST FxRequest;

} WORKITEM_CONTEXT, * PWORKITEM_CONTEXT;


//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext);
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT, GetRequestContext);
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(MEMORY_CONTEXT, GetMemoryContext);
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WORKITEM_CONTEXT, GetWorkItemContext);

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO, _HidGetIdleCallbackInfo);//原先缺失，后续添加

typedef enum HID_COMMAND
{
    CmdMin = 0,
    CmdReservedLow = CmdMin,
    CmdReset = 1,
    CmdGetReport = 2,
    CmdSetReport = 3,
    CmdGetIdle = 4,
    CmdSetIdle = 5,
    CmdGetProtocol = 6,
    CmdSetProtocol = 7,
    CmdSetPower = 8,
    CmdReservedHigh = 9,
    CmdMax = CmdReservedHigh
} HID_COMMAND, * PHID_COMMAND;

// Report Types
typedef enum HID_REPORT_TYPE
{
    ReportTypeMin = 0,
    ReportTypeReserved = ReportTypeMin,
    ReportTypeInput = 1,
    ReportTypeOutput = 2,
    ReportTypeFeature = 3,
    ReportTypeMax = ReportTypeFeature
} HID_REPORT_TYPE, * PHID_REPORT_TYPE;

// Power States
typedef enum HID_POWER_STATE
{
    PowerStateMin = 0,
    PowerStateOn = PowerStateMin,
    PowerStateSleep = 1,
    PowerStateMax = PowerStateSleep
} HID_POWER_STATE, * PHID_POWER_STATE;


//
// HID specific defines
const USHORT HID_DESCRIPTOR_SIZE_V1 = 0x1E;
const USHORT HID_DESCRIPTOR_BCD_VERSION = 0x0100;
const USHORT HID_DESCRIPTOR_LENGTH_FIELD_SIZE = 2;
const USHORT HID_REPORT_LENGTH_FIELD_SIZE = 2;
const USHORT HID_REPORT_ID_OPTIMAL = 15;

const PWSTR MANUFACTURER_ID_STRING = L"Microsoft";
const PWSTR PRODUCT_ID_STRING = L"HIDI2C Device";
const PWSTR SERIAL_NUMBER_STRING = L"9999";

//
// Registers 
//
const USHORT REG_SIZE = 2;

//
// ACPI defines/constants

// {3CDFF6F7-4267-4555-AD05-B30A3D8938DE}
DEFINE_GUID(ACPI_DSM_GUID_HIDI2C,
    0x3CDFF6F7, 0x4267, 0x4555, 0xAD, 0x05, 0xB3, 0x0A, 0x3D, 0x89, 0x38, 0xDE);
//#define ACPI_DSM_GUID_HIDI2C  {3CDFF6F7-4267-4555-AD05-B30A3D8938DE}


#define E_POINTER 0x80004003

#define ACPI_DSM_REVISION               1
#define ACPI_DSM_QUERY_FUNCTION         0
#define ACPI_DSM_HIDI2C_FUNCTION        1
#define ACPI_DSM_INPUT_ARGUMENTS_COUNT  4

#define EXPECTED_IOCTL_ACPI_GET_DEVICE_INFORMATION_STRING_LENGTH 100

// Default timeout for a Request sent to a IO Target 1 second
const ULONGLONG HIDI2C_REQUEST_DEFAULT_TIMEOUT = 1000;//原先单位为秒太大改为ms
//
// Tag used for pool allocations.
#define HIDI2C_POOL_TAG 'IdiH'

#define HIDI2C_ALLOCATE_POOL(Type, Size) \
    ExAllocatePoolWithTag((Type), (Size), HIDI2C_POOL_TAG)

#define HIDI2C_FREE_POOL(Buffer) \
    ExFreePoolWithTag((Buffer), HIDI2C_POOL_TAG)

#define MOUSEREPORT_BUFFER_SIZE  sizeof(mouse_report_t)


#define VHID_HARDWARE_IDS    L"HID\\hidi2c_filter\0\0"
#define VHID_HARDWARE_IDS_LENGTH sizeof (VHID_HARDWARE_IDS)

#define IOCTL_INTERNAL_MOUSE_REPORT CTL_CODE(FILE_DEVICE_MOUSE,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)//自定义ioctl

#define FUNCTION_FROM_CTL_CODE(ctrlCode) (((ULONG)((ctrlCode) & 0x3FFC)) >> 2)

ULONG runtimes_hid = 0;
ULONG runtimes_OnInterruptIsr = 0;
ULONG runtimes_OnInterruptDpc = 0;
ULONG runtimes_OnPostInterruptsEnabled = 0;
ULONG runtimes_OnSelfManagedIoSuspend = 0;
ULONG runtimes_ioControl = 0;
ULONG runtimes_HidEvtResetTimerFired = 0;


VOID RegDebug(WCHAR* strValueName, PVOID dataValue, ULONG datasizeValue);
VOID RegDebugInt(WCHAR* strValueName, int dNum, PVOID dataValue, ULONG datasizeValue);

NTSTATUS
RegistryGetDeviceSettings(
    _In_  WDFDEVICE                     FxDevice,
    _Out_ PREGISTRY_DEVICE_SETTINGS     Settings);

EXTERN_C DRIVER_INITIALIZE                  DriverEntry;
EXTERN_C EVT_WDF_DRIVER_DEVICE_ADD          OnDeviceAdd;
EXTERN_C EVT_WDF_DRIVER_UNLOAD              OnDriverUnload;


// WDF Device callback events
EVT_WDF_DEVICE_PREPARE_HARDWARE                     OnPrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE                     OnReleaseHardware;
EVT_WDF_DEVICE_D0_ENTRY                             OnD0Entry;
EVT_WDF_DEVICE_D0_EXIT                              OnD0Exit;
EVT_WDF_DEVICE_D0_ENTRY_POST_INTERRUPTS_ENABLED     OnPostInterruptsEnabled;
NTSTATUS OnSelfManagedIoSuspend(WDFDEVICE Device);//PFN_WDF_DEVICE_SELF_MANAGED_IO_SUSPEND              OnSelfManagedIoSuspend;


// WDF Interrupt callback events
EVT_WDF_INTERRUPT_ISR           OnInterruptIsr;
EVT_WDF_INTERRUPT_WORKITEM      OnInterruptWorkItem;
EVT_WDF_INTERRUPT_DPC           OnInterruptDpc;

//
// ETW tracing event helpers
//

GUID
_LogGetActivityId(
    _In_opt_    WDFREQUEST FxRequest
);

VOID
PowerIdleIrpCompletion(
    _In_ PDEVICE_CONTEXT    FxDeviceContext
);

EVT_WDF_WORKITEM PowerIdleIrpWorkitem;

// WDF Queue callback events
//
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL OnInternalDeviceIoControl;
EVT_WDF_IO_QUEUE_IO_STOP                    OnIoStop;


//
EXTERN_C
NTSTATUS
AcpiInitialize(
    _In_ PDEVICE_CONTEXT    FxDeviceContext
);

EXTERN_C
NTSTATUS
AcpiGetDeviceInformation(
    _In_ PDEVICE_CONTEXT    FxDeviceContext
);

EXTERN_C
NTSTATUS
AcpiGetDeviceMethod(
    _In_ PDEVICE_CONTEXT    FxDeviceContext
);

EXTERN_C
NTSTATUS
AcpiDsmSupported(
    _In_ PDEVICE_CONTEXT    FxDeviceContext,
    _In_ ULONG              FunctionIndex,
    _Out_ PBOOLEAN          Supported
);

EXTERN_C
VOID
AcpiPrepareInputParametersForDsm(
    _Inout_ PACPI_EVAL_INPUT_BUFFER_COMPLEX ParametersBuffer,
    _In_ ULONG ParametersBufferSize,
    _In_ ULONG FunctionIndex
);



//
// Public Functions

NTSTATUS
SpbInitialize(
    _In_ PDEVICE_CONTEXT    FxDeviceContext,
    _In_ WDFCMRESLIST       FxResourcesRaw,
    _In_ WDFCMRESLIST       FxResourcesTranslated
);

VOID
SpbDestroy(
    _In_ PDEVICE_CONTEXT  FxDeviceContext
);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
SpbWrite(
    _In_                    WDFIOTARGET FxIoTarget,
    _In_                    USHORT      RegisterAddress,
    _In_reads_(DataLength)  PBYTE       Data,
    _In_                    ULONG       DataLength,
    _In_                    ULONG       Timeout = HIDI2C_REQUEST_DEFAULT_TIMEOUT
);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
SpbWritelessRead(
    _In_                        WDFIOTARGET FxIoTarget,
    _In_                        WDFREQUEST  FxRequest,
    _Out_writes_(DataLength)    PBYTE       Data,
    _In_                        ULONG       DataLength
);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
SpbRead(
    _In_                        WDFIOTARGET FxIoTarget,
    _In_                        USHORT      RegisterAddress,
    _Out_writes_(DataLength)    PBYTE       Data,
    _In_                        ULONG       DataLength,
    _In_                        ULONG       DelayUs,
    _In_                        ULONG       Timeout = HIDI2C_REQUEST_DEFAULT_TIMEOUT
);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
SpbWriteWrite(
    _In_                            WDFIOTARGET FxIoTarget,
    _In_                            USHORT      RegisterAddressFirst,
    _In_reads_(DataLengthFirst)     PBYTE       DataFirst,
    _In_                            USHORT      DataLengthFirst,
    _In_                            USHORT      RegisterAddressSecond,
    _In_reads_(DataLengthSecond)    PBYTE       DataSecond,
    _In_                            USHORT      DataLengthSecond
);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
SpbWriteRead(
    _In_                            WDFIOTARGET     FxIoTarget,
    _In_                            USHORT          RegisterAddressFirst,
    _In_reads_(DataLengthFirst)     PBYTE           DataFirst,
    _In_                            USHORT          DataLengthFirst,
    _In_                            USHORT          RegisterAddressSecond,
    _Out_writes_(DataLengthSecond)  PBYTE           DataSecond,
    _In_                            USHORT          DataLengthSecond,
    _In_                            ULONG           DelayUs
);

//
// Private Functions
//

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
SpbSequence(
    _In_                        WDFIOTARGET     FxIoTarget,
    _In_reads_(SequenceLength)  PVOID           Sequence,
    _In_                        SIZE_T          SequenceLength,
    _Out_                       PULONG          BytesReturned,
    _In_                        ULONG           Timeout = HIDI2C_REQUEST_DEFAULT_TIMEOUT
);




NTSTATUS HidInitialize(PDEVICE_CONTEXT pDevContext, WDF_POWER_DEVICE_STATE  FxPreviousState);

NTSTATUS HidPower(PDEVICE_CONTEXT pDevContext, SHORT PowerState);

void HidEvtResetTimerFired(WDFTIMER Timer);

NTSTATUS HidReset(PDEVICE_CONTEXT pDevContext);

NTSTATUS HidDestroy(PDEVICE_CONTEXT pDevContext, WDF_POWER_DEVICE_STATE FxTargetState);

NTSTATUS HidSendIdleNotification(
    PDEVICE_CONTEXT pDevContext,
    WDFREQUEST FxRequest,
    BOOLEAN* bRequestPendingFlag);

NTSTATUS HidGetHidDescriptor(PDEVICE_CONTEXT pDevContext);

NTSTATUS
HidGetDeviceAttributes(
    PDEVICE_CONTEXT pDevContext,
    WDFREQUEST Request
);

NTSTATUS
HidGetDeviceDescriptor(
    PDEVICE_CONTEXT pDevContext,
    WDFREQUEST Request
);

NTSTATUS
HidGetReportDescriptor(
    PDEVICE_CONTEXT pDevContext,
    WDFREQUEST Request
);

NTSTATUS
HidGetString(
    PDEVICE_CONTEXT pDevContext,
    WDFREQUEST Request
);

NTSTATUS
HidWriteReport(
    PDEVICE_CONTEXT pDevContext,
    WDFREQUEST Request
);

NTSTATUS
HidReadReport(
    PDEVICE_CONTEXT pDevContext,
    WDFREQUEST Request,
    BOOLEAN* requestPendingFlag
);

NTSTATUS
HidGetReport(
    PDEVICE_CONTEXT pDevContext,
    WDFREQUEST Request,
    HID_REPORT_TYPE ReportType
);

NTSTATUS
HidSetReport(
    PDEVICE_CONTEXT pDevContext,
    WDFREQUEST Request,
    HID_REPORT_TYPE ReportType
);


//鼠标HID描述符, 两个按键（左，右）， 滚轮（水平滚动和垂直滚动）, X,Y采用相对值
#define REPORTID_MOUSE  0x02
#define REPORTID_FEATURE 0x03
#define REPORTID_TOUCHPAD 0x04
#define REPORTID_MAX_COUNT 0x05
#define REPORTID_PTPHQA 0x08
#define REPORTID_STANDARDMOUSE 0x02
#define REPORTID_MULTITOUCH 0x05
#define REPORTID_REPORTMODE 0x04
#define REPORTID_FUNCTION_SWITCH 0x06   
#define REPORTID_DEVICE_CAPS 0x07
#define REPORTID_UMAPP_CONF  0x09


#define REPORTID_STANDARDMOUSE 0x02
#define REPORTID_FUNCSWITCH 0x06

#define BUTTON_SWITCH 0x57
#define SURFACE_SWITCH 0x58

#define USAGE_PAGE 0x05
#define USAGE_PAGE_1 0x06
#define USAGE      0x09
#define USAGE_MINIMUM 0x19
#define USAGE_MAXIMUM 0x29
#define LOGICAL_MINIMUM 0x15
#define LOGICAL_MAXIMUM 0x25
#define LOGICAL_MAXIMUM_2 0x26
#define LOGICAL_MAXIMUM_3 0x27
#define PHYSICAL_MINIMUM 0x35
#define PHYSICAL_MAXIMUM 0x45
#define PHYSICAL_MAXIMUM_2 0x46
#define PHYSICAL_MAXIMUM_3 0x47
#define UNIT_EXPONENT 0x55
#define UNIT 0x65
#define UNIT_2 0x66

#define REPORT_ID       0x85
#define REPORT_COUNT    0x95
#define REPORT_COUNT_2	0x96
#define REPORT_SIZE     0x75
#define INPUT           0x81
#define FEATURE         0xb1

#define BEGIN_COLLECTION 0xa1
#define END_COLLECTION   0xc0


#define DEVICE_VERSION 0x01
#define MAX_FINGERS	16

#define MouseReportDescriptor_TLC \
	0x05, 0x01,\
	0x09, 0x02,\
	0xA1, 0x01,\
	0x09, 0x01,\
		0xA1, 0x00,\
		0x85, REPORTID_MOUSE,\
		0x05, 0x09,\
		0x19, 0x01,\
		0x29, 0x03,\
		0x15, 0x00,\
		0x25, 0x01,\
		0x95, 0x03,\
		0x75, 0x01,\
		0x81, 0x02,\
		0x95, 0x01,\
		0x75, 0x05,\
		0x81, 0x03,\
		0x05, 0x01,\
		0x09, 0x30,\
		0x09, 0x31,\
		0x09, 0x38,\
		0x15, 0x81,\
		0x25, 0x7F,\
		0x75, 0x08,\
		0x95, 0x03,\
		0x81, 0x06,\
		0x05, 0x0C,\
		0x0A, 0x38, 0x02, \
		0x15, 0x81,\
		0x25, 0x7F,\
		0x75, 0x08,\
		0x95, 0x01,\
		0x81, 0x06,\
		0xC0,0xC0,\


#define AAPL_PTP_USERMODE_CONFIGURATION_APP_TLC \
	USAGE_PAGE_1, 0x00, 0xff, /* Usage Page: Vendor defined */ \
	USAGE, 0x01, /* Usage: Vendor Usage 0x01 */ \
	BEGIN_COLLECTION, 0x01, /* Begin Collection: Application */ \
		REPORT_ID, REPORTID_UMAPP_CONF, /* Report ID: User-mode Application configuration */ \
		USAGE, 0x01, /* Usage: Vendor Usage 0x01 */ \
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum 0 */ \
		LOGICAL_MAXIMUM_2, 0xff, 0x00, /* Logical Maximum 255 */ \
		REPORT_SIZE, 0x08, /* Report Size: 8 */ \
		REPORT_COUNT, 0x03, /* Report Count: 3 */ \
		FEATURE, 0x02, /* Feature: (Data, Var, Abs) */ \
	END_COLLECTION

#define AAPL_PTP_WINDOWS_CONFIGURATION_TLC \
	USAGE_PAGE, 0x0d, /* Usage Page: Digitizer */ \
	USAGE, 0x0e, /* Usage: Configuration */ \
	BEGIN_COLLECTION, 0x01, /* Begin Collection: Application */ \
		REPORT_ID, REPORTID_REPORTMODE, /* Report ID: Mode Selection */ \
		USAGE, 0x22, /* Usage: Finger */ \
		BEGIN_COLLECTION, 0x02, /* Begin Collection: Logical */ \
			USAGE, 0x52, /* Usage: Input Mode */ \
			LOGICAL_MINIMUM, 0x00, /* Logical Minumum: 0 finger */ \
			LOGICAL_MAXIMUM, MAX_FINGERS, /* Logical Maximum: MAX_TOUCH_COUNT fingers */ \
			REPORT_SIZE, 0x08, /* Report Size: 0x08 */ \
			REPORT_COUNT, 0x01, /* Report Count: 0x01 */ \
			FEATURE, 0x02, /* Feature: (Data, Var, Abs) */ \
		END_COLLECTION, /* End Collection */ \
		BEGIN_COLLECTION, 0x00, /* Begin Collection: Physical */ \
			REPORT_ID, REPORTID_FUNCSWITCH, /* Report ID: Function Switch */ \
			USAGE, BUTTON_SWITCH, /* Usage: Button Switch */ \
			USAGE, SURFACE_SWITCH, /* Usage: Surface Switch */ \
			REPORT_SIZE, 0x01, /* Report Size: 0x01 */ \
			REPORT_COUNT, 0x02, /* Report Count: 0x02 */ \
			LOGICAL_MAXIMUM, 0x01, /* Logical Maximum: 0x01 */ \
			FEATURE, 0x02, /* Feature: (Data, Var, Abs) */ \
			REPORT_COUNT, 0x06, /* Report Count: 0x06 */ \
			FEATURE, 0x03, /* Feature: (Const, Var, Abs) */ \
		END_COLLECTION, /* End Collection */ \
	END_COLLECTION /* End Collection */

#define DEFAULT_PTP_HQA_BLOB \
	0xfc, 0x28, 0xfe, 0x84, 0x40, 0xcb, 0x9a, 0x87, \
	0x0d, 0xbe, 0x57, 0x3c, 0xb6, 0x70, 0x09, 0x88, \
	0x07, 0x97, 0x2d, 0x2b, 0xe3, 0x38, 0x34, 0xb6, \
	0x6c, 0xed, 0xb0, 0xf7, 0xe5, 0x9c, 0xf6, 0xc2, \
	0x2e, 0x84, 0x1b, 0xe8, 0xb4, 0x51, 0x78, 0x43, \
	0x1f, 0x28, 0x4b, 0x7c, 0x2d, 0x53, 0xaf, 0xfc, \
	0x47, 0x70, 0x1b, 0x59, 0x6f, 0x74, 0x43, 0xc4, \
	0xf3, 0x47, 0x18, 0x53, 0x1a, 0xa2, 0xa1, 0x71, \
	0xc7, 0x95, 0x0e, 0x31, 0x55, 0x21, 0xd3, 0xb5, \
	0x1e, 0xe9, 0x0c, 0xba, 0xec, 0xb8, 0x89, 0x19, \
	0x3e, 0xb3, 0xaf, 0x75, 0x81, 0x9d, 0x53, 0xb9, \
	0x41, 0x57, 0xf4, 0x6d, 0x39, 0x25, 0x29, 0x7c, \
	0x87, 0xd9, 0xb4, 0x98, 0x45, 0x7d, 0xa7, 0x26, \
	0x9c, 0x65, 0x3b, 0x85, 0x68, 0x89, 0xd7, 0x3b, \
	0xbd, 0xff, 0x14, 0x67, 0xf2, 0x2b, 0xf0, 0x2a, \
	0x41, 0x54, 0xf0, 0xfd, 0x2c, 0x66, 0x7c, 0xf8, \
	0xc0, 0x8f, 0x33, 0x13, 0x03, 0xf1, 0xd3, 0xc1, \
	0x0b, 0x89, 0xd9, 0x1b, 0x62, 0xcd, 0x51, 0xb7, \
	0x80, 0xb8, 0xaf, 0x3a, 0x10, 0xc1, 0x8a, 0x5b, \
	0xe8, 0x8a, 0x56, 0xf0, 0x8c, 0xaa, 0xfa, 0x35, \
	0xe9, 0x42, 0xc4, 0xd8, 0x55, 0xc3, 0x38, 0xcc, \
	0x2b, 0x53, 0x5c, 0x69, 0x52, 0xd5, 0xc8, 0x73, \
	0x02, 0x38, 0x7c, 0x73, 0xb6, 0x41, 0xe7, 0xff, \
	0x05, 0xd8, 0x2b, 0x79, 0x9a, 0xe2, 0x34, 0x60, \
	0x8f, 0xa3, 0x32, 0x1f, 0x09, 0x78, 0x62, 0xbc, \
	0x80, 0xe3, 0x0f, 0xbd, 0x65, 0x20, 0x08, 0x13, \
	0xc1, 0xe2, 0xee, 0x53, 0x2d, 0x86, 0x7e, 0xa7, \
	0x5a, 0xc5, 0xd3, 0x7d, 0x98, 0xbe, 0x31, 0x48, \
	0x1f, 0xfb, 0xda, 0xaf, 0xa2, 0xa8, 0x6a, 0x89, \
	0xd6, 0xbf, 0xf2, 0xd3, 0x32, 0x2a, 0x9a, 0xe4, \
	0xcf, 0x17, 0xb7, 0xb8, 0xf4, 0xe1, 0x33, 0x08, \
	0x24, 0x8b, 0xc4, 0x43, 0xa5, 0xe5, 0x24, 0xc2


#define AAPL_SPI_SERIES3_13_PTP_FINGER_COLLECTION_1 \
	BEGIN_COLLECTION, 0x02, /* Begin Collection: Logical */ \
		/* Begin a byte */ \
		LOGICAL_MAXIMUM, 0x01, /* Logical Maximum: 1 */ \
		USAGE, 0x47, /* Usage: Confidence */ \
		USAGE, 0x42, /* Usage: Tip switch */ \
		REPORT_COUNT, 0x02, /* Report Count: 2 */ \
		REPORT_SIZE, 0x01, /* Report Size: 1 */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_COUNT, 0x01, /* Report Count: 1 */ \
		REPORT_SIZE, 0x03, /* Report Size: 3 */ \
		LOGICAL_MAXIMUM, 0x03, /* Logical Maximum: 3 */ \
		USAGE, 0x51, /* Usage: Contract Identifier */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_SIZE, 0x01, /* Report Size: 1 */ \
		REPORT_COUNT, 0x03, /* Report Count: 3 */ \
		INPUT, 0x03, /* Input: (Const, Var, Abs) */ \
		/* End of a byte */ \
		/* Begin of 4 bytes */ \
		/* Size is hard-coded at this moment */ \
		/* This hard-coded size is designed for MacBookPro 13 / 14 */ \
		USAGE_PAGE, 0x01, /* Usage Page: Generic Desktop */ \
		LOGICAL_MAXIMUM_2, 0xc0, 0x32, /* Logical Maximum: 12992 (See defintion) */ \
		REPORT_SIZE, 0x10, /* Report Size: 0x10 (2 bytes) */ \
		UNIT_EXPONENT, 0x0e, /* Unit exponent: -2 */ \
		UNIT, 0x11, /* Unit: SI Length (cm) */ \
		USAGE, 0x30, /* Usage: X */ \
		PHYSICAL_MAXIMUM_2, 0x48, 0x05, /* Physical Maximum: 1352 (See Apple Spec) */ \
		REPORT_COUNT, 0x01, /* Report count: 1 */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		PHYSICAL_MAXIMUM_2, 0x44, 0x03, /* Physical Maximum: 836 (See Apple Spec) */ \
		LOGICAL_MAXIMUM_2, 0xaf, 0x1e, /* Logical Maximum: 7855 (See definition) */ \
		USAGE, 0x31, /* Usage: Y */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		PHYSICAL_MAXIMUM, 0x00, /* Physical Maximum: 0 */ \
		UNIT_EXPONENT, 0x00, /* Unit exponent: 0 */ \
		UNIT, 0x00, /* Unit: None */ \
		/* End of 4 bytes */ \
	END_COLLECTION /* End Collection */ \

#define AAPL_SPI_SERIES3_13_PTP_FINGER_COLLECTION_2 \
	BEGIN_COLLECTION, 0x02, /* Begin Collection: Logical */ \
		/* Begin a byte */ \
		LOGICAL_MAXIMUM, 0x01, /* Logical Maximum: 1 */ \
		USAGE, 0x47, /* Usage: Confidence */ \
		USAGE, 0x42, /* Usage: Tip switch */ \
		REPORT_COUNT, 0x02, /* Report Count: 2 */ \
		REPORT_SIZE, 0x01, /* Report Size: 1 */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_COUNT, 0x01, /* Report Count: 1 */ \
		REPORT_SIZE, 0x03, /* Report Size: 3 */ \
		LOGICAL_MAXIMUM, 0x03, /* Logical Maximum: 3 */ \
		USAGE, 0x51, /* Usage: Contract Identifier */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_SIZE, 0x01, /* Report Size: 1 */ \
		REPORT_COUNT, 0x03, /* Report Count: 3 */ \
		INPUT, 0x03, /* Input: (Const, Var, Abs) */ \
		/* End of a byte */ \
		/* Begin of 4 bytes */ \
		/* Size is hard-coded at this moment */ \
		USAGE_PAGE, 0x01, /* Usage Page: Generic Desktop */ \
		LOGICAL_MAXIMUM_2, 0xc0, 0x32, /* Logical Maximum: 12992 (See defintion) */ \
		REPORT_SIZE, 0x10, /* Report Size: 0x10 (2 bytes) */ \
		UNIT_EXPONENT, 0x0e, /* Unit exponent: -2 */ \
		UNIT, 0x11, /* Unit: SI Length (cm) */ \
		USAGE, 0x30, /* Usage: X */ \
		PHYSICAL_MAXIMUM_2, 0x48, 0x05, /* Physical Maximum: 1352 (See Apple Spec) */ \
		REPORT_COUNT, 0x01, /* Report count: 1 */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		PHYSICAL_MAXIMUM_2, 0x44, 0x03, /* Physical Maximum: 836 (See Apple Spec) */ \
		LOGICAL_MAXIMUM_2, 0xaf, 0x1e, /* Logical Maximum: 7855 (See definition) */ \
		USAGE, 0x31, /* Usage: Y */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		/* End of 4 bytes */ \
	END_COLLECTION /* End Collection */ \

#define AAPL_SPI_SERIES3_13_PTP_TLC \
	USAGE_PAGE, 0x0d, /* Usage Page: Digitizer */ \
	USAGE, 0x05, /* Usage: Touch Pad */ \
	BEGIN_COLLECTION, 0x01, /* Begin Collection: Application */ \
		REPORT_ID, REPORTID_MULTITOUCH, /* Report ID: Multi-touch */ \
		USAGE, 0x22, /* Usage: Finger */ \
		AAPL_SPI_SERIES3_13_PTP_FINGER_COLLECTION_1, /* 1 */ \
		USAGE_PAGE, 0x0d, /* Usage Page: Digitizer */ \
		USAGE, 0x22, /* Usage: Finger */ \
		AAPL_SPI_SERIES3_13_PTP_FINGER_COLLECTION_1, /* 2 */ \
		USAGE_PAGE, 0x0d, /* Usage Page: Digitizer */ \
		USAGE, 0x22, /* Usage: Finger */ \
		AAPL_SPI_SERIES3_13_PTP_FINGER_COLLECTION_2, /* 3 */ \
		USAGE_PAGE, 0x0d, /* Usage Page: Digitizer */ \
		USAGE, 0x22, /* Usage: Finger */ \
		AAPL_SPI_SERIES3_13_PTP_FINGER_COLLECTION_1, /* 4 */ \
		USAGE_PAGE, 0x0d, /* Usage Page: Digitizer */ \
		USAGE, 0x22, /* Usage: Finger */ \
		AAPL_SPI_SERIES3_13_PTP_FINGER_COLLECTION_2, /* 5 */ \
		USAGE_PAGE, 0x0d, /* Usage Page: Digitizer */ \
		UNIT_EXPONENT, 0x0c, /* Unit exponent: -4 */ \
		UNIT_2, 0x01, 0x10, /* Time: Second */ \
		PHYSICAL_MAXIMUM_3, 0xff, 0xff, 0x00, 0x00, \
		LOGICAL_MAXIMUM_3, 0xff, 0xff, 0x00, 0x00, \
		USAGE, 0x56, /* Usage: Scan Time */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x54, /* Usage: Contact Count */ \
		LOGICAL_MAXIMUM, 0x7f, \
		REPORT_SIZE, 0x08, \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE_PAGE, 0x09, /* Usage Page: Button */ \
		USAGE, 0x01, /* Button 1 */ \
		LOGICAL_MAXIMUM, 0x01, \
		REPORT_SIZE, 0x01, \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_COUNT, 0x07, \
		INPUT, 0x03, /* Input: (Const, Var, Abs) */ \
		USAGE_PAGE, 0x0d, /* Usage Page: Digitizer */ \
		REPORT_ID, REPORTID_DEVICE_CAPS, \
		USAGE, 0x55, /* Usage: Maximum Contacts */ \
		USAGE, 0x59, /* Usage: Touchpad Button Type*/ \
		LOGICAL_MINIMUM, 0x00, \
		LOGICAL_MAXIMUM_2, 0xff, 0x00, \
		REPORT_SIZE, 0x08, \
		REPORT_COUNT, 0x02, \
		FEATURE, 0x02, \
		USAGE_PAGE_1, 0x00, 0xff, \
		REPORT_ID, REPORTID_PTPHQA, \
		USAGE, 0xc5, \
		LOGICAL_MINIMUM, 0x00, \
		LOGICAL_MAXIMUM_2, 0xff, 0x00, \
		REPORT_SIZE, 0x08, \
		REPORT_COUNT_2, 0x00, 0x01, \
		FEATURE, 0x02, \
	END_COLLECTION /* End Collection */


const unsigned char AmtPtpSpiFamily3aReportDescriptor[] = {
    AAPL_SPI_SERIES3_13_PTP_TLC,
    AAPL_PTP_WINDOWS_CONFIGURATION_TLC,
    AAPL_PTP_USERMODE_CONFIGURATION_APP_TLC,
    MouseReportDescriptor_TLC
};


const unsigned char TouchpadReportDescriptor[] = {//关键问题是每个REPORTID_必须不同以区分报告类别，并且值在1-255之间
    //TOUCH PAD input TLC
        0x05, 0x0d,                         // USAGE_PAGE (Digitizers)          
        0x09, 0x05,                         // USAGE (Touch Pad)             
        0xa1, 0x01,                         // COLLECTION (Application)         
        0x85, REPORTID_MULTITOUCH,            //   REPORT_ID (Touch pad)      //REPORTID_MULTITOUCH       REPORTID_TOUCHPAD               
        0x09, 0x22,                         //   USAGE (Finger)                 
        0xa1, 0x02,                         //   COLLECTION (Logical)  
        0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
        0x25, 0x01,                         //       LOGICAL_MAXIMUM (1)
        0x09, 0x47,                         //       USAGE (Confidence) 
        0x09, 0x42,                         //       USAGE (Tip switch)
        0x95, 0x02,                         //       REPORT_COUNT (2)
        0x75, 0x01,                         //       REPORT_SIZE (1)
        0x81, 0x02,                         //       INPUT (Data,Var,Abs)
        0x95, 0x01,                         //       REPORT_COUNT (1)
        0x75, 0x02,                         //       REPORT_SIZE (2)
        0x25, 0x02,                         //       LOGICAL_MAXIMUM (2)
        0x09, 0x51,                         //       USAGE (Contact Identifier)
        0x81, 0x02,                         //       INPUT (Data,Var,Abs)
        0x75, 0x01,                         //       REPORT_SIZE (1)
        0x95, 0x04,                         //       REPORT_COUNT (4)             
        0x81, 0x03,                         //       INPUT (Cnst,Var,Abs)
        0x05, 0x01,                         //       USAGE_PAGE (Generic Desk..
        0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
        0x26, 0xff, 0x0f,                   //       LOGICAL_MAXIMUM (4095)         
        0x75, 0x10,                         //       REPORT_SIZE (16)             
        0x55, 0x0e,                         //       UNIT_EXPONENT (-2)           
        0x65, 0x13,                         //       UNIT(Inch,EngLinear)                  
        0x09, 0x30,                         //       USAGE (X)                    
        0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)         
        0x46, 0x90, 0x01,                   //       PHYSICAL_MAXIMUM (400)
        0x95, 0x01,                         //       REPORT_COUNT (1)         
        0x81, 0x02,                         //       INPUT (Data,Var,Abs)         
        0x46, 0x13, 0x01,                   //       PHYSICAL_MAXIMUM (275)
        0x09, 0x31,                         //       USAGE (Y)                    
        0x81, 0x02,                         //       INPUT (Data,Var,Abs)    
        0xc0,                               //    END_COLLECTION
        0x55, 0x0C,                         //    UNIT_EXPONENT (-4)           
        0x66, 0x01, 0x10,                   //    UNIT (Seconds)        
        0x47, 0xff, 0xff, 0x00, 0x00,      //     PHYSICAL_MAXIMUM (65535)
        0x27, 0xff, 0xff, 0x00, 0x00,         //  LOGICAL_MAXIMUM (65535) 
        0x75, 0x10,                           //  REPORT_SIZE (16)             
        0x95, 0x01,                           //  REPORT_COUNT (1) 
        0x05, 0x0d,                         //    USAGE_PAGE (Digitizers)
        0x09, 0x56,                         //    USAGE (Scan Time)    
        0x81, 0x02,                           //  INPUT (Data,Var,Abs)         
        0x09, 0x54,                         //    USAGE (Contact count)
        0x25, 0x7f,                           //  LOGICAL_MAXIMUM (127) 
        0x95, 0x01,                         //    REPORT_COUNT (1)
        0x75, 0x08,                         //    REPORT_SIZE (8)    
        0x81, 0x02,                         //    INPUT (Data,Var,Abs)
        0x05, 0x09,                         //    USAGE_PAGE (Button)         
        0x09, 0x01,                         //    USAGE_(Button 1)     
        0x25, 0x01,                         //    LOGICAL_MAXIMUM (1)          
        0x75, 0x01,                         //    REPORT_SIZE (1)              
        0x95, 0x01,                         //    REPORT_COUNT (1)             
        0x81, 0x02,                         //    INPUT (Data,Var,Abs)
        0x95, 0x07,                          //   REPORT_COUNT (7)                 
        0x81, 0x03,                         //    INPUT (Cnst,Var,Abs)
        0x05, 0x0d,                         //    USAGE_PAGE (Digitizer)
        0x85, REPORTID_DEVICE_CAPS,            //   REPORT_ID (Feature)  /硬件支持点数    REPORTID_DEVICE_CAPS，REPORTID_MAX_COUNT,                   
        0x09, 0x55,                         //    USAGE (Contact Count Maximum)
        0x09, 0x59,                         //    USAGE (Pad TYpe)
        0x75, 0x04,                         //    REPORT_SIZE (4) 
        0x95, 0x02,                         //    REPORT_COUNT (2)
        0x25, 0x0f,                         //    LOGICAL_MAXIMUM (15)
        0xb1, 0x02,                         //    FEATURE (Data,Var,Abs)
        0x06, 0x00, 0xff,                   //    USAGE_PAGE (Vendor Defined)
        0x85, REPORTID_PTPHQA,               //    REPORT_ID (PTPHQA)  
        0x09, 0xC5,                         //    USAGE (Vendor Usage 0xC5)    
        0x15, 0x00,                         //    LOGICAL_MINIMUM (0)          
        0x26, 0xff, 0x00,                   //    LOGICAL_MAXIMUM (0xff) 
        0x75, 0x08,                         //    REPORT_SIZE (8)             
        0x96, 0x00, 0x01,                   //    REPORT_COUNT (0x100 (256))             
        0xb1, 0x02,                         //    FEATURE (Data,Var,Abs)
        0xc0,                               // END_COLLECTION
        //CONFIG TLC
        0x05, 0x0d,                         //    USAGE_PAGE (Digitizer)
        0x09, 0x0E,                         //    USAGE (Configuration)
        0xa1, 0x01,                         //   COLLECTION (Application)
        0x85, REPORTID_REPORTMODE,             //   REPORT_ID (Feature)      REPORTID_FEATURE       
        0x09, 0x22,                         //   USAGE (Finger)              
        0xa1, 0x02,                         //   COLLECTION (logical)     
        0x09, 0x52,                         //    USAGE (Input Mode)         
        0x15, 0x00,                         //    LOGICAL_MINIMUM (0)      
        0x25, 0x0a,                         //    LOGICAL_MAXIMUM (10)
        0x75, 0x08,                         //    REPORT_SIZE (8)         
        0x95, 0x01,                         //    REPORT_COUNT (1)         
        0xb1, 0x02,                         //    FEATURE (Data,Var,Abs    
        0xc0,                               //   END_COLLECTION
        0x09, 0x22,                         //   USAGE (Finger)              
        0xa1, 0x00,                         //   COLLECTION (physical)     
        0x85, REPORTID_FUNCTION_SWITCH,     //     REPORT_ID (Feature)              
        0x09, 0x57,                         //     USAGE(Surface switch)
        0x09, 0x58,                         //     USAGE(Button switch)
        0x75, 0x01,                         //     REPORT_SIZE (1)
        0x95, 0x02,                         //     REPORT_COUNT (2)
        0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
        0xb1, 0x02,                         //     FEATURE (Data,Var,Abs)
        0x95, 0x06,                         //     REPORT_COUNT (6)             
        0xb1, 0x03,                         //     FEATURE (Cnst,Var,Abs)
        0xc0,                               //   END_COLLECTION
        0xc0,                               // END_COLLECTION
        //MOUSE TLC
        0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     
        0x09, 0x02,                         // USAGE (Mouse)                    
        0xa1, 0x01,                         // COLLECTION (Application)        
        0x85, REPORTID_MOUSE,               //   REPORT_ID (Mouse)              
        0x09, 0x01,                         //   USAGE (Pointer)                
        0xa1, 0x00,                         //   COLLECTION (Physical)          
        0x05, 0x09,                         //     USAGE_PAGE (Button)          
        0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     
        0x29, 0x02,                         //     USAGE_MAXIMUM (Button 2)     
        0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          
        0x75, 0x01,                         //     REPORT_SIZE (1)              
        0x95, 0x02,                         //     REPORT_COUNT (2)             
        0x81, 0x02,                         //     INPUT (Data,Var,Abs)         
        0x95, 0x06,                         //     REPORT_COUNT (6)             
        0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         
        0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 
        0x09, 0x30,                         //     USAGE (X)                    
        0x09, 0x31,                         //     USAGE (Y)                    
        0x75, 0x10,                         //     REPORT_SIZE (16)             
        0x95, 0x02,                         //     REPORT_COUNT (2)             
        0x25, 0x0a,                          //    LOGICAL_MAXIMUM (10)      
        0x81, 0x06,                         //     INPUT (Data,Var,Rel)         
        0xc0,                               //   END_COLLECTION                 
        0xc0,                                //END_COLLECTION      

};

const unsigned char DefaultPtpReportDescriptor[] = {
    // MOUSE TLC
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     
    0x09, 0x02,                         // USAGE (Mouse)                    
    0xa1, 0x01,                         // COLLECTION (Application)        
    0x85, REPORTID_MOUSE,               //   REPORT_ID (Mouse)              
    0x09, 0x01,                         //   USAGE (Pointer)                
    0xa1, 0x00,                         //   COLLECTION (Physical)          
    0x05, 0x09,                         //     USAGE_PAGE (Button)          
    0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     
    0x29, 0x02,                         //     USAGE_MAXIMUM (Button 2)   
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1) 
    0x75, 0x01,                         //     REPORT_SIZE (1)              
    0x95, 0x02,                         //     REPORT_COUNT (2)    
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)    
    0x95, 0x06,                         //     REPORT_COUNT (6) 
    0x81, 0x01,                         //     INPUT (Constant)
    0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 
    0x09, 0x30,                         //     USAGE (X)                    
    0x09, 0x31,                         //     USAGE (Y)         
    0x15, 0x81,                         //     LOGICAL_MINIMUM (129)   
    0x25, 0x7f,                          //    LOGICAL_MAXIMUM (127)   
    0x75, 0x08,                         //     REPORT_SIZE (8) 
    0x95, 0x02,                         //     REPORT_COUNT (2)   
    0x81, 0x06,                         //     INPUT (Data,Var,Rel)
    0xc0,                               //   END_COLLECTION                 
    0xc0,                                //END_COLLECTION 

        //TOUCH PAD input TLC
    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)          
    0x09, 0x05,                         // USAGE (Touch Pad)    
    0xa1, 0x01,                         // COLLECTION (Application)         
    0x85, REPORTID_FEATURE,            //   REPORT_ID   ??
    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)   
    0x09, 0x22,                         //   USAGE (Finger)                 
    0xa1, 0x02,                         //   COLLECTION (Logical)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)  
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1) 
    0x09, 0x47,                         //       USAGE (Confidence) 
    0x09, 0x42,                         //       USAGE (Tip switch)
    0x95, 0x02,                         //       REPORT_COUNT (2)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x75, 0x03,                         //       REPORT_SIZE (3)  
    0x25, 0x05,                         //       LOGICAL_MAXIMUM (5) 
    0x09, 0x51,                         //       USAGE (Contact Identifier)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x95, 0x03,                         //       REPORT_COUNT (3)  
    0x81, 0x03,                         //       INPUT (Constant, Var)
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)    
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x26, 0x7c, 0x05,                   //       LOGICAL_MAXIMUM (1404)
    0x75, 0x10,                         //       REPORT_SIZE (16)             
    0x55, 0x0e,                         //       UNIT_EXPONENT (-2)     
    0x65, 0x11,                         //       UNIT(cm厘米)   
    0x09, 0x30,                         //     USAGE (X) 
    0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)         
    0x46, 0x90, 0x04,                   //       PHYSICAL_MAXIMUM (1168)  
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x46, 0xD0, 0x02,                   //       PHYSICAL_MAXIMUM (720)
    0x26, 0x60, 0x03,                   //       LOGICAL_MAXIMUM (864) 
    0x09, 0x31,                         //     USAGE (Y)      
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0xc0,                               //   END_COLLECTION   

    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)   
    0x09, 0x22,                         //   USAGE (Finger) 
    0xa1, 0x00,                         //   COLLECTION (Physical)   
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)  ?
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1) 
    0x09, 0x47,                         //       USAGE (Confidence) 
    0x09, 0x42,                         //       USAGE (Tip switch)
    0x95, 0x02,                         //       REPORT_COUNT (2)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x75, 0x03,                         //       REPORT_SIZE (3)  
    0x25, 0x05,                         //       LOGICAL_MAXIMUM (5) 
    0x09, 0x51,                         //       USAGE (Contact Identifier)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x95, 0x03,                         //       REPORT_COUNT (3)  
    0x81, 0x03,                         //       INPUT (Constant,Var)
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x26, 0x7c, 0x05,                   //       LOGICAL_MAXIMUM (1404)
    0x75, 0x10,                         //       REPORT_SIZE (16)             
    0x55, 0x0e,                         //       UNIT_EXPONENT (-2)   
    0x65, 0x11,                         //       UNIT(cm厘米)   
    0x09, 0x30,                         //     USAGE (X) 
    0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)         
    0x46, 0x90, 0x04,                   //       PHYSICAL_MAXIMUM (1168)  
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x46, 0xD0, 0x02,                   //       PHYSICAL_MAXIMUM (720) 
    0x26, 0x60, 0x03,                   //       LOGICAL_MAXIMUM (864) 
    0x09, 0x31,                         //     USAGE (Y)      
    0x81, 0x02,                         //       INPUT (Data,Var)
    0xc0,                               //   END_COLLECTION   

    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)   
    0x09, 0x22,                         //   USAGE (Finger) 
    0xa1, 0x02,                         //   COLLECTION (Logical)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)  
    0x25, 0x05,                         //       LOGICAL_MAXIMUM (5) 
    0x09, 0x51,                         //       USAGE (Contact Identifier)
    0x81, 0x02,                         //       INPUT (Data,Var)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x95, 0x03,                         //       REPORT_COUNT (3)  
    0x81, 0x03,                         //       INPUT (Constant,Var)
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x26, 0x7c, 0x05,                   //       LOGICAL_MAXIMUM (1404)
    0x75, 0x10,                         //       REPORT_SIZE (16)             
    0x55, 0x0e,                         //       UNIT_EXPONENT (-2)   
    0x65, 0x11,                         //       UNIT(cm厘米)   
    0x09, 0x30,                         //     USAGE (X) 
    0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)         
    0x46, 0x90, 0x04,                   //       PHYSICAL_MAXIMUM (1168)  
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x46, 0xD0, 0x02,                   //       PHYSICAL_MAXIMUM (720) 
    0x26, 0x60, 0x03,                   //       LOGICAL_MAXIMUM (864)  
    0x09, 0x31,                         //     USAGE (Y)      
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0xc0,                               //   END_COLLECTION   

    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)   
    0x09, 0x22,                         //   USAGE (Finger) 
    0xa1, 0x02,                         //   COLLECTION (Logical)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0) 
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
    0x09, 0x47,                         //       USAGE (Confidence) 
    0x09, 0x42,                         //       USAGE (Tip switch)
    0x95, 0x02,                         //       REPORT_COUNT (2)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x81, 0x02,                         //       INPUT (Data,Var)
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x75, 0x03,                         //       REPORT_SIZE (3)  
    0x25, 0x05,                         //       LOGICAL_MAXIMUM (5) 
    0x09, 0x51,                         //       USAGE (Contact Identifier)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x95, 0x03,                         //       REPORT_COUNT (3)  
    0x81, 0x03,                         //       INPUT (Constant,Var)
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x26, 0x7c, 0x05,                   //       LOGICAL_MAXIMUM (1404) 
    0x75, 0x10,                         //       REPORT_SIZE (16)             
    0x55, 0x0e,                         //       UNIT_EXPONENT (-2)   
    0x65, 0x11,                         //       UNIT(cm厘米)   
    0x09, 0x30,                         //     USAGE (X) 
    0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)         
    0x46, 0x90, 0x04,                   //       PHYSICAL_MAXIMUM (1168) 
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x46, 0xD0, 0x02,                   //       PHYSICAL_MAXIMUM (720) 
    0x26, 0x60, 0x03,                   //       LOGICAL_MAXIMUM (864) 
    0x09, 0x31,                         //     USAGE (Y)      
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0xc0,                               //   END_COLLECTION   

    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)   
    0x09, 0x22,                         //   USAGE (Finger) 
    0xa1, 0x02,                         //   COLLECTION (Logical)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)  
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
    0x09, 0x47,                         //       USAGE (Confidence) 
    0x09, 0x42,                         //       USAGE (Tip switch)
    0x95, 0x02,                         //       REPORT_COUNT (2)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x75, 0x03,                         //       REPORT_SIZE (3) 
    0x25, 0x05,                         //       LOGICAL_MAXIMUM (5) 
    0x09, 0x51,                         //       USAGE (Contact Identifier)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x75, 0x01,                         //       REPORT_SIZE (1)
    0x95, 0x03,                         //       REPORT_COUNT (2)  ?
    0x81, 0x03,                         //       INPUT (Data,Var,Abs)  ?
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)  
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
    0x26, 0x7c, 0x05,                   //       LOGICAL_MAXIMUM (1404)
    0x75, 0x10,                         //       REPORT_SIZE (16)             
    0x55, 0x0e,                         //       UNIT_EXPONENT (-2)   
    0x65, 0x11,                         //       UNIT(cm厘米)   
    0x09, 0x30,                         //     USAGE (X) 
    0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)         
    0x46, 0x90, 0x04,                   //       PHYSICAL_MAXIMUM (1168)   
    0x95, 0x01,                         //       REPORT_COUNT (1)
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x46, 0xD0, 0x02,                   //       PHYSICAL_MAXIMUM (720) 
    0x26, 0x60, 0x03,                   //       LOGICAL_MAXIMUM (864) 
    0x09, 0x31,                         //     USAGE (Y)      
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0xc0,                               //   END_COLLECTION   

    0x05, 0x0d,                         // USAGE_PAGE (Digitizers) 
    0x55, 0x0C,                         //    UNIT_EXPONENT (-4) 
    0x66, 0x01, 0x10,                   //    UNIT (Seconds)        
    0x47, 0xff, 0xff, 0x00, 0x00,      //     PHYSICAL_MAXIMUM (65535)
    0x27, 0xff, 0xff, 0x00, 0x00,         //  LOGICAL_MAXIMUM (65535) 
    0x75, 0x10,                           //  REPORT_SIZE (16)             
    0x95, 0x01,                           //  REPORT_COUNT (1) 

    0x09, 0x56,                         //    USAGE (Scan Time)    
    0x81, 0x02,                           //  INPUT (Data,Var,Abs)         
    0x09, 0x54,                         //    USAGE (Contact count)
    0x25, 0x7f,                           //  LOGICAL_MAXIMUM (127) 
    0x95, 0x01,                         //    REPORT_COUNT (1)
    0x75, 0x08,                         //    REPORT_SIZE (8)    
    0x81, 0x02,                         //    INPUT (Data,Var,Abs)
    0x05, 0x09,                         //    USAGE_PAGE (Button)         
    0x09, 0x01,                         //    USAGE_(Button 1)     
    0x25, 0x01,                         //    LOGICAL_MAXIMUM (1)          
    0x75, 0x01,                         //    REPORT_SIZE (1)              
    0x95, 0x01,                         //    REPORT_COUNT (1)             
    0x81, 0x02,                         //    INPUT (Data,Var,Abs)
    0x95, 0x07,                          //   REPORT_COUNT (7)                 
    0x81, 0x03,                         //    INPUT (Constant,Var)

    0x05, 0x0d,                         //    USAGE_PAGE (Digitizer)
    0x85, REPORTID_PTPHQA,            //    REPORT_ID (PTPHQA)                  
    0x09, 0x55,                         //    USAGE (Contact Count Maximum)
    0x09, 0x59,                         //    USAGE (Pad TYpe)
    0x75, 0x04,                         //    REPORT_SIZE (4) 
    0x95, 0x02,                         //    REPORT_COUNT (2)
    0x25, 0x0f,                         //    LOGICAL_MAXIMUM (15)
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs)
    0x85, 0x0D,               //    REPORT_ID   Latency mode feature report id
    0x09, 0x60,                         //    USAGE (  Latency mode feature report ) 
    0x75, 0x01,                         //    REPORT_SIZE (1)              
    0x95, 0x01,                         //    REPORT_COUNT (1)    
    0x15, 0x00,                         //       LOGICAL_MINIMUM (0) 
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs)
    0x95, 0x07,                          //   REPORT_COUNT (7)  

    0xb1, 0x03,                         //    FEATURE (Constant,Var)   
    0x85, REPORTID_DEVICE_CAPS,            //   REPORT_ID (Feature)  /硬件支持点数
    0x06, 0x00, 0xff,                   //    USAGE_PAGE (Vendor Defined)
    0x09, 0xC5,                         //    USAGE (Vendor Usage 0xC5)    
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)          
    0x26, 0xff, 0x00,                   //    LOGICAL_MAXIMUM (0xff) 
    0x75, 0x08,                         //    REPORT_SIZE (8)             
    0x96, 0x00, 0x01,                   //    REPORT_COUNT (0x100 (256))     
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs)
    0xc0,                               // END_COLLECTION

//CONFIG TLC
    0x05, 0x0d,                         //    USAGE_PAGE (Digitizer)
    0x09, 0x0E,                         //    USAGE (Configuration)
    0xa1, 0x01,                         //   COLLECTION (Application)
    0x85, REPORTID_REPORTMODE,             //   REPORT_ID (Feature)      REPORTID_FEATURE       
    0x09, 0x22,                         //   USAGE (Finger)              
    0xa1, 0x02,                         //   COLLECTION (logical)     
    0x09, 0x52,                         //    USAGE (Input Mode)         
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)      
    0x25, 0x0a,                         //    LOGICAL_MAXIMUM (10)
    0x75, 0x08,                         //    REPORT_SIZE (8)         
    0x95, 0x01,                         //    REPORT_COUNT (1)         
    0xb1, 0x02,                         //    FEATURE (Data,Var,Abs    
    0xc0,                               //   END_COLLECTION

    0x09, 0x22,                         //   USAGE (Finger)              
    0xa1, 0x00,                         //   COLLECTION (physical)     
    0x85, REPORTID_FUNCTION_SWITCH,     //     REPORT_ID (Feature)              
    0x09, 0x57,                         //     USAGE(Surface switch)
    0x09, 0x58,                         //     USAGE(Button switch)
    0x75, 0x01,                         //     REPORT_SIZE (1)
    0x95, 0x02,                         //     REPORT_COUNT (2)
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
    0xb1, 0x02,                         //     FEATURE (Data,Var,Abs)
    0x95, 0x06,                         //     REPORT_COUNT (6)             
    0xb1, 0x03,                         //     FEATURE (Cnst,Var,Abs)
    0xc0,                               //   END_COLLECTION
    0xc0,                               // END_COLLECTION

//Vendor Defined
    0x06, 0x00, 0xff,                   //     USAGE_PAGE (Vendor Defined)  
    0x95, 0x01,                         //     REPORT_COUNT (1) 
    0xa1, 0x01,                         //   COLLECTION (Application)
    0x85, 0x09,              //     REPORT_ID ( )        ??//Button 2	Indicates Button State for external button for primary (default left) clicking
    0x09, 0x02,                         // USAGE (Button)    ??//Button 2	Indicates Button State for external button for primary (default left) clicking
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                   //     LOGICAL_MAXIMUM (0xff)  
    0x75, 0x08,                         //     REPORT_SIZE (8)
    0x95, 0x14,                         //     REPORT_COUNT (20) 

    0x09, 0xC6,                         //     USAGE (Vendor usage for segment #) 
    0x25, 0x08,                         //     LOGICAL_MAXIMUM (8)
    0x75, 0x08,                         //     REPORT_SIZE (8)
    0x95, 0x01,                         //     REPORT_COUNT (1) 
    0xb1, 0x02,                         //     FEATURE (Data,Var,Abs) 
    0x09, 0xC7,                         //     USAGE (Vendor Usage) 
    0x26, 0xff, 0x00,                   //     LOGICAL_MAXIMUM (0xff)                 
    0x95, 0x20,                         //     REPORT_COUNT (32)             
    0xb1, 0x02,                         //     FEATURE (Data,Var,Abs)

    0x91, 0x02,                         //这个字段是输出用的，用来控制LED。变量，值，绝对值。
    0x85, 0x0a,     //     REPORT_ID ( )   ??
    0x09, 0x03,                         //   USAGE(Button)   ??//Button 3	Indicates Button State for external button for secondary (default right) clicking
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                   //     LOGICAL_MAXIMUM (0xff)  
    0x75, 0x08,                         //     REPORT_SIZE (8)
    0x95, 0x14,                         //     REPORT_COUNT (20)  

    0x91, 0x02,                         //这个字段是输出用的，用来控制LED。变量，值，绝对值。
    0x85, 0x0b,     //     REPORT_ID ( )   ??

    0x09, 0x04,                        //   USAGE(Button)   ??//Button 4	Indicates Button State for external button for   clicking  
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                   //     LOGICAL_MAXIMUM (0xff)  
    0x75, 0x08,                         //     REPORT_SIZE (8)
    0x95, 0x3d,                         //     REPORT_COUNT (61)  
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x85, 0x0c,     //     REPORT_ID ( )   ??

    0x09, 0x05,                          //   USAGE(Button)   ??//Button 5	Indicates Button State for external button for   clicking  
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                   //     LOGICAL_MAXIMUM (0xff)  
    0x75, 0x08,                         //     REPORT_SIZE (8)
    0x95, 0x3d,                         //     REPORT_COUNT (61)  
    0x81, 0x02,                         //       INPUT (Data,Var,Abs)
    0x85, 0x0f,     //     REPORT_ID ( )   ??

    0x09, 0x06,                          //   USAGE(Button)   ??//Button 6	Indicates Button State for external button for   clicking  
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                   //     LOGICAL_MAXIMUM (0xff)  
    0x75, 0x08,                         //     REPORT_SIZE (8)
    0x95, 0x03,                         //     REPORT_COUNT (3)  
    0xb1, 0x02,                         //     FEATURE (Data,Var,Abs)
    0x85, 0x0e,     //     REPORT_ID ( )   ??

    0x09, 0x07,                          //   USAGE(Button)   ??//Button 7	Indicates Button State for external button for   clicking  
    0x15, 0x00,                         //    LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                   //     LOGICAL_MAXIMUM (0xff)  
    0x75, 0x08,                         //     REPORT_SIZE (8)
    0x95, 0x01,                         //     REPORT_COUNT (1)
    0xb1, 0x02,                         //     FEATURE (Data,Var,Abs)
    0xc0,                               // END_COLLECTION
};

///////
const unsigned char MouseReportDescriptor[] = {
    ///
    0x05, 0x01, // USAGE_PAGE(Generic Desktop)
    0x09, 0x02, //   USAGE(Mouse)
    0xA1, 0x01, //   COLLECTION(APPlication)
    0x09, 0x01, //   USAGE(Pointer)
        0xA1, 0x00, //     COLLECTION(Physical)
        0x85, REPORTID_MOUSE, //     ReportID(Mouse ReportID)
        0x05, 0x09, //     USAGE_PAGE(Button)
        0x19, 0x01, //     USAGE_MINIMUM(button 1)   Button 按键， 位 0 左键， 位1 右键， 位2 中键
        0x29, 0x03, //     USAGE_MAXMUM(button 3)  //0x03限制最大的鼠标按键数量
        0x15, 0x00, //     LOGICAL_MINIMUM(0)
        0x25, 0x01, //     LOGICAL_MAXIMUM(1)
        0x95, 0x03, //     REPORT_COUNT(3)  //0x03鼠标按键数量
        0x75, 0x01, //     REPORT_SIZE(1)
        0x81, 0x02, //     INPUT(Data,Var,Abs)
        0x95, 0x01, //     REPORT_COUNT(1)
        0x75, 0x05, //     REPORT_SIZE(5)  //需要补足多少个bit使得加上鼠标按键数量的3个bit位成1个字节8bit
        0x81, 0x03, //     INPUT(Data,Var, Abs)
        0x05, 0x01, //     USAGE_PAGE(Generic Desktop)
        0x09, 0x30, //     USAGE(X)       X移动
        0x09, 0x31, //     USAGE(Y)       Y移动
        0x09, 0x38, //     USAGE(Wheel)   垂直滚动
        0x15, 0x81, //     LOGICAL_MINIMUM(-127)
        0x25, 0x7F, //     LOGICAL_MAXIMUM(127)
        0x75, 0x08, //     REPORT_SIZE(8)
        0x95, 0x03, //     REPORT_COUNT(3)
        0x81, 0x06, //     INPUT(Data,Var, Rel) //X,Y,垂直滚轮三个参数， 相对值

        //下边水平滚动
        0x05, 0x0C, //     USAGE_PAGE (Consumer Devices)
        0x0A, 0x38, 0x02, // USAGE(AC Pan)
        0x15, 0x81, //       LOGICAL_MINIMUM(-127)
        0x25, 0x7F, //       LOGICAL_MAXIMUM(127)
        0x75, 0x08, //       REPORT_SIZE(8)
        0x95, 0x01, //       REPORT_COUNT(1)
        0x81, 0x06, //       INPUT(data,Var, Rel) //水平滚轮，相对值
        0xC0,       //       End Connection(PhySical)
    0xC0,       //     End Connection

};


CONST HID_DESCRIPTOR DefaultHidDescriptor = {
    0x09,   // length of HID descriptor
    0x21,   // descriptor type == HID  0x21
    0x0100, // hid spec release
    0x00,   // country code == Not Specified
    0x01,   // number of HID class descriptors
    { 0x22,   // descriptor type 
    sizeof(MouseReportDescriptor) }  // MouseReportDescriptor//TouchpadReportDescriptor//AmtPtpSpiFamily3aReportDescriptor
};


///鼠标状态报告,对应的HID是上边的报告
#pragma pack(1)
struct mouse_report_t
{
    BYTE    report_id;
    BYTE    button; //0 no press, 1 left, 2 right ; 3 左右同时按下，触摸板一般不会有这种事
    CHAR    dx;
    CHAR    dy;
    CHAR    v_wheel; // 垂直
    CHAR    h_wheel; // 水平
};
#pragma pack()



#define PTP_MAX_CONTACT_POINTS 5
#define PTP_BUTTON_TYPE_CLICK_PAD 0
#define PTP_BUTTON_TYPE_PRESSURE_PAD 1

#define PTP_COLLECTION_MOUSE 0
#define PTP_COLLECTION_WINDOWS 3

#define PTP_FEATURE_INPUT_COLLECTION   0
#define PTP_FEATURE_SELECTIVE_REPORTING   1
#define PTP_SELECTIVE_REPORT_Button_Surface_ON 3

#define PTP_CONTACT_CONFIDENCE_BIT   1
#define PTP_CONTACT_TIPSWITCH_BIT    2


// HID report descriptor constants
#define HID_TYPE_COLLECTION 0xA1
#define HID_TYPE_END_COLLECTION 0xC0
#define HID_TYPE_USAGE_PAGE 0x05
#define HID_TYPE_USAGE 0x09
#define HID_TYPE_REPORT_ID 0x85
#define HID_TYPE_REPORT_SIZE 0x75
#define HID_TYPE_REPORT_COUNT 0x95
#define HID_TYPE_FEATURE 0xB1
#define HID_USAGE_PAGE_DIGITIZERS 0x0D
#define HID_USAGE_TOUCHPAD 0x05
#define HID_USAGE_TOUCHSCREEN 0x04
#define HID_USAGE_CONFIGURATION 0x0E

#define HID_USAGE_INPUT_MODE 0x52
#define HID_USAGE_X 0x30
#define HID_USAGE_Y 0x31

typedef struct _PTP_DEVICE_CAPS_FEATURE_REPORT {
    UCHAR ReportID;
    UCHAR MaximumContactPoints;
    UCHAR ButtonType;
} PTP_DEVICE_CAPS_FEATURE_REPORT, * PPTP_DEVICE_CAPS_FEATURE_REPORT;

typedef struct _PTP_DEVICE_HQA_CERTIFICATION_REPORT {
    UCHAR ReportID;
    UCHAR CertificationBlob[256];
} PTP_DEVICE_HQA_CERTIFICATION_REPORT, * PPTP_DEVICE_HQA_CERTIFICATION_REPORT;

typedef struct _PTP_DEVICE_INPUT_MODE_REPORT {
    UCHAR ReportID;
    UCHAR Mode;
} PTP_DEVICE_INPUT_MODE_REPORT, * PPTP_DEVICE_INPUT_MODE_REPORT;

#pragma pack(push)
#pragma pack(1)
typedef struct _PTP_DEVICE_SELECTIVE_REPORT_MODE_REPORT {
    UCHAR ReportID;
    UCHAR DeviceMode;
    UCHAR ButtonReport : 1;
    UCHAR SurfaceReport : 1;
    UCHAR Padding : 6;
} PTP_DEVICE_SELECTIVE_REPORT_MODE_REPORT, * PPTP_DEVICE_SELECTIVE_REPORT_MODE_REPORT;
#pragma pack(pop)


enum CONTACT_STATE {
    CONTACT_NEW = 0,
    CONTACT_CONTINUED = 1,
    CONTACT_CONFIDENCE_CANCELLED = 2,
    CONTACT_INVALID = 3
};




NTSTATUS
AmtPtpSetFeatures(
    _In_ WDFDEVICE Device,
    _In_ WDFREQUEST Request
);

NTSTATUS
AmtPtpReportFeatures(
    _In_ WDFDEVICE Device,
    _In_ WDFREQUEST Request
);

NTSTATUS
AmtPtpGetStrings(
    _In_ WDFDEVICE Device,
    _In_ WDFREQUEST Request,
    _Out_ BOOLEAN* Pending
);

NTSTATUS
HidSetFeature(
    PDEVICE_CONTEXT pDevContext,
    BOOLEAN SetType
);

NTSTATUS
AnalyzeHidReportDescriptor(
    PDEVICE_CONTEXT pDevContext
);

NTSTATUS
GetReportDescriptor(
    PDEVICE_CONTEXT pDevContext
);

NTSTATUS EvtPnpQueryIds(WDFDEVICE device, PIRP Irp);


NTSTATUS
SendPtpReport(PDEVICE_CONTEXT pDevContext, PTP_REPORT* pPtpReport);

NTSTATUS
SendPtpMouseReport(PDEVICE_CONTEXT pDevContext, mouse_report_t* pMouseReport);

NTSTATUS
SendMouseFilterReport(PDEVICE_CONTEXT pDevContext, mouse_report_t* mReport);

NTSTATUS
Connect2MouseFilter(
    PDEVICE_CONTEXT pDevContext
);

NTSTATUS Create_reuse_request(PDEVICE_CONTEXT pDevContext);

void MouseLikeTouchPad_parse_init(PDEVICE_CONTEXT pDevContext);

void MouseLikeTouchPad_parse(PDEVICE_CONTEXT pDevContext, PTP_REPORT* pPtpReport, mouse_report_t* pMouseReport);

