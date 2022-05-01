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
#include <usb.h>
#include <wdfusb.h>
#include <initguid.h>
#include <assert.h>

#include "device.h"
#include "queue.h"
#include "trace.h"

#define TEST_BOARD_TRANSFER_BUFFER_SIZE (64*1024)
#define EMPTY_PACKAGE (0)

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD USBUMDF2Driver2EvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP USBUMDF2Driver2EvtDriverContextCleanup;

EXTERN_C_END
