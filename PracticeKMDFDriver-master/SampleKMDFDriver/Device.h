/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include "public.h"

EXTERN_C_START

// Define the symbolic name of the device stack
// No need for device object name, it is automatically created and used
#define SYMBOLIC_NAME L"\\DosDevices\\WDM_DEVICESTACK"

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
	// Buffer to temporarily store the string sent from the application
	PVOID pInternalStringBuffer;

	// Size of the string sent by the application
	SIZE_T dwStringLength;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

//
// Function to initialize the device and its callbacks
//
NTSTATUS
SampleKMDFDriverCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

EXTERN_C_END
