/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    User-mode Driver Framework 2

--*/
//#ifdef _KERNEL_MODE
//#include <ntddk.h>
//#else
//#include <windows.h>
//#endif

#include <windows.h>
#include <wdf.h>
#include <initguid.h>

#include "device.h"
#include "queue.h"


//
//#include <hidport.h>  // located in $(DDK_INC_PATH)/wdm
#define IOCTL_HID_GET_DEVICE_DESCRIPTOR             HID_CTL_CODE(0)
#define IOCTL_HID_GET_REPORT_DESCRIPTOR             HID_CTL_CODE(1)
#define IOCTL_HID_READ_REPORT                       HID_CTL_CODE(2)
#define IOCTL_HID_WRITE_REPORT                      HID_CTL_CODE(3)
#define IOCTL_HID_GET_STRING                        HID_CTL_CODE(4)
#define IOCTL_HID_ACTIVATE_DEVICE                   HID_CTL_CODE(7)
#define IOCTL_HID_DEACTIVATE_DEVICE                 HID_CTL_CODE(8)
#define IOCTL_HID_GET_DEVICE_ATTRIBUTES             HID_CTL_CODE(9)
#define IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST    HID_CTL_CODE(10)

//
// Internal IOCTLs supported by UMDF HID minidriver.  
//
#define IOCTL_UMDF_HID_SET_FEATURE                   HID_CTL_CODE(20)
#define IOCTL_UMDF_HID_GET_FEATURE                   HID_CTL_CODE(21)
#define IOCTL_UMDF_HID_SET_OUTPUT_REPORT             HID_CTL_CODE(22)
#define IOCTL_UMDF_HID_GET_INPUT_REPORT              HID_CTL_CODE(23)
#define IOCTL_UMDF_GET_PHYSICAL_DESCRIPTOR           HID_CTL_CODE(24)

#include <hidclass.h>
//#include "hidpi.h"
//#include "hidsdi.h"   // Must link in hid.lib
//#include "hidpi.h"



EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD MouseLikeTouchPadFilterEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP MouseLikeTouchPadFilterEvtDriverContextCleanup;

EXTERN_C_END


VOID RegDebug(PDEVICE_CONTEXT pDevContext, LPCWSTR strValueName, PVOID dataValue, ULONG datasizeValue);
