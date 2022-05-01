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
USBUMDF2Driver2QueueInitialize(
    _In_ WDFDEVICE Device
    )
/*++

Routine Description:


     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG    queueConfig;
	WDF_OBJECT_ATTRIBUTES  queueAttributes;
	PQUEUE_CONTEXT queueContext;
	
    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
	

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
        WdfIoQueueDispatchParallel
        );

    queueConfig.EvtIoDeviceControl = USBUMDF2Driver2EvtIoDeviceControl;
 
	//deviceContext= DeviceGetContext(Device)
	__analysis_assume(queueConfig.EvtIoStop != 0);
    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );
	__analysis_assume(queueConfig.EvtIoStop == 0);
	

    if( !NT_SUCCESS(status) ) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", status);
		goto Error;
    }

//*****READ QUEUE
	//WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchSequential);
	WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchSequential);
	
	queueConfig.EvtIoRead = USBUMDF2Driver2EvtIoRead;
	queueConfig.EvtIoStop = USBUMDF2Driver2EvtIoStop;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queueAttributes, QUEUE_CONTEXT);
	status = WdfIoQueueCreate(
		Device,
		&queueConfig,
		&queueAttributes,
		&queue // queue handle
	);

	if (!NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "WdfIoQueueCreate failed 0x%x\n", status);
		goto Error;
	}

	status = WdfDeviceConfigureRequestDispatching(
		Device,
		queue,
		WdfRequestTypeRead);

	if (!NT_SUCCESS(status)) 
	{
		assert(NT_SUCCESS(status));
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,"WdfDeviceConfigureRequestDispatching failed 0x%x\n", status);
		goto Error;
	}
	
	//*****WRITE QUEUE
	
	//WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchSequential);
	WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchSequential);
	queueConfig.EvtIoWrite = USBUMDF2Driver2EvtIoWrite;
	queueConfig.EvtIoStop = USBUMDF2Driver2EvtIoStop;
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queueAttributes, QUEUE_CONTEXT);
	status = WdfIoQueueCreate(
		Device,
		&queueConfig,
		&queueAttributes,
		&queue // queue handle
	);

	if (!NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,
			"WdfIoQueueCreate failed 0x%x\n", status);
		goto Error;
	}

	status = WdfDeviceConfigureRequestDispatching(
		Device,
		queue,
		WdfRequestTypeWrite);

	if (!NT_SUCCESS(status)) {
		assert(NT_SUCCESS(status));
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,
			"WdfDeviceConfigureRequestDispatching failed 0x%x\n", status);
		goto Error;
	}
	queueContext = QueueGetContext(queue);

	queueContext->WriteMemory = NULL;
	queueContext->Timer = NULL;

	queueContext->CurrentRequest = NULL;
	queueContext->CurrentStatus = STATUS_INVALID_DEVICE_REQUEST;
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "QUEUE CONFIGURED 0x%x\n", status);
	return status;

Error:

	//
	// Log fail to add device to the event log
	//
	//EventWriteFailAddDevice(pDevContext->DeviceName,pDevContext->Location,status);
	TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "QUEUE ERROR %!STATUS!", status);
	
    return status;
}

VOID
USBUMDF2Driver2EvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{
    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d", 
                Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, IoControlCode);

    WdfRequestComplete(Request, STATUS_SUCCESS);

    return;
}

VOID
USBUMDF2Driver2EvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags
)
/*++

Routine Description:

    This event is invoked for a power-managed queue before the device leaves the working state (D0).

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
                  that identify the reason that the callback function is being called
                  and whether the request is cancelable.

Return Value:

    VOID

--*/
{
    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "%!FUNC! Queue 0x%p, Request 0x%p ActionFlags %d", 
                Queue, Request, ActionFlags);

    //
    // In most cases, the EvtIoStop callback function completes, cancels, or postpones
    // further processing of the I/O request.
    //
    // Typically, the driver uses the following rules:
    //
    // - If the driver owns the I/O request, it either postpones further processing
    //   of the request and calls WdfRequestStopAcknowledge, or it calls WdfRequestComplete
    //   with a completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
    //  
    //   The driver must call WdfRequestComplete only once, to either complete or cancel
    //   the request. To ensure that another thread does not call WdfRequestComplete
    //   for the same request, the EvtIoStop callback must synchronize with the driver's
    //   other event callback functions, for instance by using interlocked operations.
    //
    // - If the driver has forwarded the I/O request to an I/O target, it either calls
    //   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
    //   further processing of the request and calls WdfRequestStopAcknowledge.
    //
    // A driver might choose to take no action in EvtIoStop for requests that are
    // guaranteed to complete in a small amount of time. For example, the driver might
    // take no action for requests that are completed in one of the driver’s request handlers.
    //

    return;
}

VOID
USBUMDF2Driver2EvtIoWrite(
	_In_ WDFQUEUE         Queue,
	_In_ WDFREQUEST       Request,
	_In_ size_t           Length
)
{
	NTSTATUS                    status=0;
	WDFUSBPIPE                  pipe;
	WDFMEMORY                   reqMemory;
	PDEVICE_CONTEXT             deviceContext;
	//PUCHAR  buffer_read = NULL;
	//ULONG sizeInbuffer ;
	WDF_REQUEST_SEND_OPTIONS options;
	
	
	//_Analysis_assume_(Length > 0); 

	
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "USBUMDF2Driver2EvtIoWrite received a lenght %d argument\n",(int) Length);


	if (Length > TEST_BOARD_TRANSFER_BUFFER_SIZE) 
	{
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "Transfer exceeds %d\n",
			TEST_BOARD_TRANSFER_BUFFER_SIZE);
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}
	deviceContext = DeviceGetContext(WdfIoQueueGetDevice(Queue));
	pipe = deviceContext->BulkWritePipe;

	status = WdfRequestRetrieveInputMemory(Request, &reqMemory);
	
	

	if (!NT_SUCCESS(status)) 
	{
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "WdfMemoryCopyFromBuffer failed \n");
		WdfRequestComplete(Request, status);
		return;
	}

	//buffer_read = WdfMemoryGetBuffer(reqMemory, NULL);
	//sizeInbuffer = sizeof(reqMemory);
	
	//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,"Wr %d\n", sizeInbuffer);
		
	status = WdfUsbTargetPipeFormatRequestForWrite(pipe,Request,reqMemory,NULL); // Offset

	if (!NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,
			"WdfUsbTargetPipeFormatRequestForWrite failed 0x%x\n", status);
		goto Exit;
	}
	//Set a completion callback function
	WdfRequestSetCompletionRoutine(Request,EvtRequestWriteCompletionRoutine,pipe);

	//
	// Send the request asynchronously.
	//
	
	
	


	WDF_REQUEST_SEND_OPTIONS_INIT(&options,0); 
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&options, WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&options, WDF_REL_TIMEOUT_IN_SEC(3));
	
	if (WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pipe), WDF_NO_SEND_OPTIONS) == FALSE) {
	//if (WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pipe), &options) == FALSE) {
		//
		// Framework couldn't send the request for some reason.
		//
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "WdfRequestSend failed\n");
		status = WdfRequestGetStatus(Request);
		goto Exit;
	}
Exit:

	if (!NT_SUCCESS(status)) {
		//
		// log event write failed
		//
		//EventWriteWriteFail(WdfIoQueueGetDevice(Queue), status);

		WdfRequestCompleteWithInformation(Request, status, 0);
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "USBUMDF2Driver2EvtIoWrite\n");

	return;
}
VOID
EvtRequestWriteCompletionRoutine(
	_In_ WDFREQUEST                  Request,
	_In_ WDFIOTARGET                 Target,
	_In_ PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
	_In_ WDFCONTEXT                  Context
)
{
	NTSTATUS    status;
	size_t      bytesWritten = 0;
	PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;

	UNREFERENCED_PARAMETER(Target);
	UNREFERENCED_PARAMETER(Context);

	status = CompletionParams->IoStatus.Status;

	usbCompletionParams = CompletionParams->Parameters.Usb.Completion;

	bytesWritten = usbCompletionParams->Parameters.PipeWrite.Length;

	if (NT_SUCCESS(status)) 
	{
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, 
			"Number of bytes written: %I64d\n", (INT64)bytesWritten);
	}
	else 
	{
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,
			"Write failed: request Status 0x%x UsbdStatus 0x%x\n",
			status, usbCompletionParams->UsbdStatus);
	}
	/*EventWriteWriteStop(WdfIoQueueGetDevice(WdfRequestGetIoQueue(Request)),
		bytesWritten,
		status,
		usbCompletionParams->UsbdStatus); */


	WdfRequestCompleteWithInformation(Request, status, bytesWritten);
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "WRITE COMPLETED\n");
	return;

}

VOID
USBUMDF2Driver2EvtIoRead(
	_In_ WDFQUEUE         Queue,
	_In_ WDFREQUEST       Request,
	_In_ size_t           Length
)
{
	WDFUSBPIPE                  pipe;
	NTSTATUS                    status;
	WDFMEMORY                   reqMemory;
	PDEVICE_CONTEXT             deviceContext;
	WDF_REQUEST_SEND_OPTIONS	options;
	//PQUEUE_CONTEXT queueContext = QueueGetContext(Queue);

	//UNREFERENCED_PARAMETER(Queue);

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,  "-->USBUMDF2Driver2EvtIoRead\n");

	//
	// First validate input parameters.
	//
	if (Length > TEST_BOARD_TRANSFER_BUFFER_SIZE) {
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "Transfer exceeds %d\n",
			TEST_BOARD_TRANSFER_BUFFER_SIZE);
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	deviceContext = DeviceGetContext(WdfIoQueueGetDevice(Queue));

	pipe = deviceContext->BulkReadPipe;

	status = WdfRequestRetrieveOutputMemory(Request, &reqMemory); //Retrive a handle to the framework memory object from the IO request
	
	


	if (!NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,
			"WdfRequestRetrieveOutputMemory failed %!STATUS!\n", status);
		goto Exit;
	}
	
	//WdfRequestCompleteWithInformation(Request, status, 10);
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "WdfRequestRetrieveOutputMemory passed 0x %x\n", status);
	status = WdfUsbTargetPipeFormatRequestForRead(pipe,
		Request,
		reqMemory,
		NULL // Offsets
	);
	if (!NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,
			"WdfUsbTargetPipeFormatRequestForRead failed 0x%x\n", status);
		goto Exit;
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,"WdfUsbTargetPipeFormatRequestForRead passed 0x%x\n", status);
	WdfRequestSetCompletionRoutine(
		Request,
		EvtRequestReadCompletionRoutine,
		pipe);
	//
	// Send the request asynchronously.
	//
	WDF_REQUEST_SEND_OPTIONS_INIT(&options, 0);
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&options, WDF_REL_TIMEOUT_IN_SEC(5));
	//if (WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pipe), WDF_NO_SEND_OPTIONS) == FALSE) {
	if (WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pipe), &options) == FALSE) {
		//
		// Framework couldn't send the request for some reason.
		//
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "WdfRequestSend failed\n");
		status = WdfRequestGetStatus(Request);
		goto Exit;
	}

Exit:
	if (!NT_SUCCESS(status)) {
		//
		// log event read failed
		//
		//EventWriteReadFail(WdfIoQueueGetDevice(Queue), status);
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "<-- USBUMDF2Driver2EvtIoRead FAILED\n");
		WdfRequestCompleteWithInformation(Request, status, 0);
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "<-- USBUMDF2Driver2EvtIoRead\n");

	return;


}

VOID
EvtRequestReadCompletionRoutine(
	_In_ WDFREQUEST                  Request,
	_In_ WDFIOTARGET                 Target,
	_In_ PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
	_In_ WDFCONTEXT                  Context
)
{
	NTSTATUS    status;
	size_t      bytesRead = 0;
	PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;
	
	
	
	
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "EvtRequestReadCompletionRouting start: \n"); 

	UNREFERENCED_PARAMETER(Target);
	UNREFERENCED_PARAMETER(Context);
	//UNREFERENCED_PARAMETER(CompletionParams);
	//UNREFERENCED_PARAMETER(Request);
	
	status = CompletionParams->IoStatus.Status;
	

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "CompletionParams->IoStatus.Status \n");
	usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "CompletionParams->Parameters.Usb.Completion \n");
	

	bytesRead = usbCompletionParams->Parameters.PipeRead.Length;
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "usbCompletionParams->Parameters.PipeRead.Length %I64d\n", (INT64)bytesRead);
	if (NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,"Number of bytes read: %I64d\n", (INT64)bytesRead);
		//TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "Read data 0x%x\n", (char)usbCompletionParams);
	}
	else {
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE,
			"Read failed - request status 0x%x UsbdStatus 0x%x\n",
			status, usbCompletionParams->UsbdStatus);

	}
	
	

	//EventWriteReadStop(WdfIoQueueGetDevice(WdfRequestGetIoQueue(Request)),
	//	bytesRead,
	//	status,
	//	usbCompletionParams->UsbdStatus);
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "Exit EvtRequestReadCompletionRoutine\n");

	//WdfRequestCompleteWithInformation(Request, status, 10);
	WdfRequestCompleteWithInformation(Request, status, bytesRead);

	return;
}
