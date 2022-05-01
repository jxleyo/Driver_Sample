/*++

Module Name:

    queue.h

Abstract:

    This file contains the queue definitions.

Environment:

    User-mode Driver Framework 2

--*/

EXTERN_C_START

//
// This is the context that can be placed per queue
// and would contain per queue information.
//
typedef struct _QUEUE_CONTEXT {
    ULONG	PrivateDeviceData;  // just a placeholder
	ULONG   UsbDeviceTraits;
	WDFMEMORY WriteMemory; // Here we allocate a buffer from a test write so it can be read back
	WDFTIMER   Timer; // Timer DPC for this queue
	WDFREQUEST  CurrentRequest; // Virtual I/O
	NTSTATUS   CurrentStatus;

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

typedef struct _WINUSB_SETUP_PACKET {
	UCHAR  RequestType;
	UCHAR  Request;
	USHORT Value;
	USHORT Index;
	USHORT Length;
} WINUSB_SETUP_PACKET, * PWINUSB_SETUP_PACKET;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WINUSB_SETUP_PACKET, WinusbSetupPacketContext)

NTSTATUS
USBUMDF2Driver2QueueInitialize(
    _In_ WDFDEVICE Device
    );

//
// Events from the IoQueue object
//
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL USBUMDF2Driver2EvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_STOP USBUMDF2Driver2EvtIoStop;
EVT_WDF_IO_QUEUE_IO_WRITE USBUMDF2Driver2EvtIoWrite;
EVT_WDF_IO_QUEUE_IO_READ USBUMDF2Driver2EvtIoRead;
EVT_WDF_REQUEST_COMPLETION_ROUTINE EvtRequestWriteCompletionRoutine;
EVT_WDF_REQUEST_COMPLETION_ROUTINE EvtRequestReadCompletionRoutine;

EXTERN_C_END
