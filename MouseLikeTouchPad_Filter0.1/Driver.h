/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    User-mode Driver Framework 2

--*/

#include <windows.h>
#include <wdf.h>
#include <initguid.h>

#include "device.h"
#include "queue.h"
#include "trace.h"


//
#include <hidclass.h>
#include "hidsdi.h"   // Must link in hid.lib
#include "hidpi.h"



EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD MouseLikeTouchPadFilterEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP MouseLikeTouchPadFilterEvtDriverContextCleanup;

EXTERN_C_END


VOID RegDebug(LPCWSTR strValueName, PVOID dataValue, ULONG datasizeValue);
