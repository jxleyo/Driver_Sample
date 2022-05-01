/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"
#include "driver.tmh"

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;

    RegDebug(L"DriverEntry start", NULL, 0);

    //
    // Initialize WPP Tracing
    //
#if UMDF_VERSION_MAJOR == 2 && UMDF_VERSION_MINOR == 0
    WPP_INIT_TRACING(MYDRIVER_TRACING_ID);
#else
    WPP_INIT_TRACING( DriverObject, RegistryPath );
#endif

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = MouseLikeTouchPadFilterEvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config,
                           MouseLikeTouchPadFilterEvtDeviceAdd
                           );

    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             &attributes,
                             &config,
                             WDF_NO_HANDLE
                             );

    if (!NT_SUCCESS(status)) {
        RegDebug(L"WdfDriverCreate Failed", NULL, status);

#if UMDF_VERSION_MAJOR == 2 && UMDF_VERSION_MINOR == 0
        WPP_CLEANUP();
#else
        WPP_CLEANUP(DriverObject);
#endif
        return status;
    }


    RegDebug(L"DriverEntry ok", NULL, status);
    return status;
}

NTSTATUS
MouseLikeTouchPadFilterEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++
Routine Description:

    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. We create and initialize a device object to
    represent a new instance of the device.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(Driver);

    status = MouseLikeTouchPadFilterCreateDevice(DeviceInit);

    RegDebug(L"MouseLikeTouchPadFilterEvtDeviceAdd ok", NULL, status);
    return status;
}

VOID
MouseLikeTouchPadFilterEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
    )
/*++
Routine Description:

    Free all the resources allocated in DriverEntry.

Arguments:

    DriverObject - handle to a WDF Driver object.

Return Value:

    VOID.

--*/
{
    UNREFERENCED_PARAMETER(DriverObject);

    RegDebug(L"MouseLikeTouchPadFilterEvtDriverContextCleanup ok", NULL, 0);

    //
    // Stop WPP Tracing
    //
#if UMDF_VERSION_MAJOR == 2 && UMDF_VERSION_MINOR == 0
    WPP_CLEANUP();
#else
    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)DriverObject));
#endif
}



VOID RegDebug(LPCWSTR strValueName, PVOID dataValue, ULONG datasizeValue)//RegDebug(L"Run debug here",pBuffer,pBufferSize);//RegDebug(L"Run debug here",NULL,0x12345678);
{
    //创建注册表项
    HKEY hKey;
    NTSTATUS status = RegCreateKey(HKEY_LOCAL_MACHINE, L"Software\\RegDebug", &hKey);
    if (status != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return;
    }


    if (dataValue == NULL) {
        //设置REG_DWORD键值
        status = RegSetValueEx(hKey, strValueName, 0, REG_DWORD, (LPBYTE)&datasizeValue, 4);
        if (!NT_SUCCESS(status))
        {
            KdPrint(("设置REG_DWORD键值失败！\n"));
        }
    }
    else {
        //设置REG_BINARY键值
        status = RegSetValueEx(hKey, strValueName, 0, REG_BINARY, dataValue, datasizeValue);
        if (!NT_SUCCESS(status))
        {
            KdPrint(("设置REG_BINARY键值失败！\n"));
        }
    }

    RegFlushKey(hKey);
    //关闭注册表句柄
    RegCloseKey(hKey);
}
