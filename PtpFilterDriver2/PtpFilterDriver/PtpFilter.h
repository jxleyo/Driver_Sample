#pragma once

#define _HIDPI_NO_FUNCTION_MACROS_
#include <ntddk.h>
#include <hidclass.h>
#include <hidpddi.h>
#include <hidpi.h>
//#define NDEBUG
//#include <debug.h>
#include <ntddmou.h>
#include <kbdmou.h>

#include <wdf.h>

#include <hidport.h>
#include <ntstrsafe.h>
#include "hidsdi.h"   // Must link in hid.lib

#include <emmintrin.h>//__m128i定义的

#include <initguid.h>  
DEFINE_GUID(GUID_DEVINTERFACE_PtpFilter,
	0xf271e39d, 0xd211, 0x4a6e, 0xa4, 0x39, 0x63, 0xe5, 0x19, 0x40, 0xd2, 0x10);// 0x745a17a0, 0x74d3, 0x11d0, 0xb6, 0xfe, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda
// {f271e39d-d211-4a6e-a439-63e51940d210}


#define LOWORD (1) ((WORD)(1))
#define HIWORD (1) ((WORD)(((DWORD) (1) >>16) & 0xFFFF))

#define DPRINT DbgPrint
#define DPRINT1 DbgPrint

#define FUNCTION_FROM_CTL_CODE(ctrlCode) (((ULONG)((ctrlCode) & 0x3FFC)) >> 2)

////
#define VHID_HARDWARE_IDS    L"HID\\PtpFilter\0\0"
#define VHID_HARDWARE_IDS_LENGTH sizeof (VHID_HARDWARE_IDS)

#define MOUHID_TAG 0x50747046u

typedef struct _DEVICE_CONTEXT
{
	WDFDEVICE	hDevice;
	WDFIOTARGET IoTarget;

	ULONG       RequestLength;
	BOOLEAN     RequestDataAvailableFlag;

	WDFQUEUE    ResetNotificationQueue;
	WDFQUEUE    ReadReportQueue;

	WDFSPINLOCK ReadLoopSpinLock;
	WDFSPINLOCK ProcessedBufferSpinLock;

	WDFREQUEST  ReuseRequest;   //重复使用的
	WDFMEMORY   OutputBuffer;
	WDFMEMORY   RequestBuffer;  // 请求的Buffer
	PVOID       ReportBuffer;  // WDFMEMORY

	SHORT       REPORT_BUFFER_SIZE;
	ULONG       ReportLength;


	PHIDP_PREPARSED_DATA PreparsedData;
	PVOID UsageListBuffer;

	HIDP_CAPS Capabilities;
	ULONG     ValueCapsLength;

	float TouchPad_Physical_Length_X;
	float TouchPad_Physical_Length_Y;

	float TouchPad_Physical_DotDistance_X;//坐标x轴物理点距
	float TouchPad_Physical_DotDistance_Y;//坐标y轴物理点距

	UCHAR MaxFingerCount;
	UCHAR LastFingerCount;
	UCHAR CurrentFingerCount;

	PVOID     CurrentRequestReportBuffer;  // 
	PVOID     LastRequestReportBuffer;  // 
	
	UCHAR     MultiTouchReportID;

} DEVICE_CONTEXT, * PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)

void EvtDriverContextCleanup(IN WDFOBJECT DriverObject);
NTSTATUS EvtDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit);

NTSTATUS EvtDeviceReleaseHardware(WDFDEVICE Device,
	WDFCMRESLIST ResourcesTranslated);

NTSTATUS
EvtDevicePrepareHardware(
	_In_ WDFDEVICE Device,
	_In_ WDFCMRESLIST ResourceList,
	_In_ WDFCMRESLIST ResourceListTranslated
);

void PtpFilterEvtIoDeviceControl(
	IN WDFQUEUE     Queue,
	IN WDFREQUEST   Request,
	IN size_t       OutputBufferLength,
	IN size_t       InputBufferLength,
	IN ULONG        IoControlCode);


NTSTATUS EvtDeviceD0Entry(WDFDEVICE Device, WDF_POWER_DEVICE_STATE PreviousState);
NTSTATUS EvtDeviceD0Exit(WDFDEVICE Device, WDF_POWER_DEVICE_STATE TargetState);

NTSTATUS Hid_StartDevice(PDEVICE_CONTEXT pDeviceContext);
VOID
Filter_DispatchPassThrough(
	_In_ WDFREQUEST Request,
	_In_ WDFIOTARGET Target
);

NTSTATUS PtpFilterSendReadRequest(PDEVICE_CONTEXT pDeviceContext, EVT_WDF_REQUEST_COMPLETION_ROUTINE PtpFilter_EvtRequestIoctlCompletionRoutine);

NTSTATUS PtpFilterBufferStoreReport(USHORT REPORT_BUFFER_SIZE, PCHAR ReportBuffer, ULONG ReportLength, UCHAR CurrentFingersCountValue);
NTSTATUS PtpFilterCheckAndStartReadLoop(PDEVICE_CONTEXT pDeviceContext, WDFREQUEST Request, EVT_WDF_REQUEST_COMPLETION_ROUTINE PtpFilter_EvtRequestIoctlCompletionRoutine);
NTSTATUS PtpFilterCompleteReadRequest(WDFREQUEST Request, PVOID SourceBuffer, ULONG_PTR Information_size);

void PtpFilterEvtIoRead(_In_ WDFQUEUE Queue, _In_ WDFREQUEST Request, _In_ size_t Length);
NTSTATUS PtpFilterGetFingersCount(PDEVICE_CONTEXT pDeviceContext, ULONG* CurrentFingersCountValue, BOOLEAN* boolFlag_FingerNotChanged);
NTSTATUS PtpFilterGetMaxFingers(PDEVICE_CONTEXT pDeviceContext);
NTSTATUS PtpFilterGetReportId(PULONG UsageValue, PDEVICE_CONTEXT pDeviceContext);

NTSTATUS PtpFilterHandleDeviceData(PDEVICE_CONTEXT pDeviceContext);
NTSTATUS  PtpFilterProcessCurrentRequest(PDEVICE_CONTEXT pDeviceContext, WDFREQUEST Request, BOOLEAN* bRequestForwardFlag);
NTSTATUS PtpFilterProcessPendingRequest(PDEVICE_CONTEXT pDeviceContext);
void PtpFilterProcessReadReport(PDEVICE_CONTEXT pDeviceContext, NTSTATUS Status_In, BOOLEAN* bRequestStopFlag);
NTSTATUS PtpFilter_WdfIoTargetFormatRequestForInternalIoctl(ULONG IoControlCode, WDFIOTARGET IoTarget, WDFMEMORY InputBuffer, ULONG InputBuffer_size, WDFMEMORY OutputBuffer, ULONG OutputBuffer_size);

double ExponentTransfor(SHORT Value);
double GetPhysicalValue(ULONG UnitsExp, ULONG Units, LONG PhysicalMax, LONG PhysicalMin);


VOID RegDebug(WCHAR* strValueName, PVOID dataValue, ULONG datasizeValue);


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

///// 30 length
typedef struct _PTP_REPORT {
	UCHAR       ReportID;
	PTP_CONTACT Contacts[5];
	USHORT      ScanTime;
	UCHAR       ContactCount;
	UCHAR       IsButtonClicked;
} PTP_REPORT, * PPTP_REPORT;


SHORT Unit_TABLE[6]= { 0, 1, 2, 3, 4, 0 }; 

LONG UnitExponent_Table[24] =
{
  5,
  5,
  6,
  6,
  7,
  7,
  8,
  -8,
  9,
  -7,
  10,
  -6,
  11,
  -5,
  12,
  -4,
  13,
  -3,
  14,
  -2,
  15,
  -1,
  0,
  0
}; 


///////
#define REPORTID_MOUSE  0x02
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
	sizeof(MouseReportDescriptor) }  // MouseReportDescriptor  TouchpadReportDescriptor
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




