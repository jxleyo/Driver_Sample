/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;

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
        return status;
    }


    return status;
}

NTSTATUS
MouseLikeTouchPadFilterEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(Driver);

    status = MouseLikeTouchPadFilterCreateDevice(DeviceInit);

    return status;
}

VOID
MouseLikeTouchPadFilterEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject);
}



VOID RegDebug(PDEVICE_CONTEXT pDevContext, LPCWSTR strValueName, PVOID dataValue, ULONG datasizeValue)//RegDebug(L"Run debug here",pBuffer,pBufferSize);//RegDebug(L"Run debug here",NULL,0x12345678);
{
    NTSTATUS status = STATUS_SUCCESS;
    WDFDEVICE device = pDevContext->FxDevice;

    //初始化valueName
    UNICODE_STRING valueName;
    RtlInitUnicodeString(&valueName, strValueName);

    WDFKEY hKey = NULL;

    status = WdfDeviceOpenRegistryKey(
        device,
        PLUGPLAY_REGKEY_DEVICE | WDF_REGKEY_DEVICE_SUBKEY,//PLUGPLAY_REGKEY_DRIVER//PLUGPLAY_REGKEY_DEVICE//
        KEY_READ,//KEY_WRITE//KEY_READ | KEY_WRITE//KEY_ALL_ACCESS
        WDF_NO_OBJECT_ATTRIBUTES,
        &hKey);

    if (status == STATUS_INVALID_DEVICE_REQUEST) {//未在 IRQL = PASSIVE_LEVEL 调用 WdfDeviceOpenRegistryKey。
        
    }
    else if (status == STATUS_INVALID_PARAMETER) {//指定了无效参数。 对于 UMDF，此返回值可能指示访问权限不足STATUS_ACCESS_DENIED。
        //MessageBox(NULL, L"失败终止", L"终止", 0);
    }
    else if (status == STATUS_INSUFFICIENT_RESOURCES) {//无法分配注册表项对象。

    }
    else if (status == STATUS_OBJECT_NAME_NOT_FOUND) {//指定的注册表项不存在。

    }

    if (!NT_SUCCESS(status)) {
        if (hKey) {
            WdfObjectDelete(hKey);
        }
        return;
    }


    DECLARE_CONST_UNICODE_STRING(myKeyStr, L"Regdebug");
    WDFKEY  subkey;

    status = WdfRegistryCreateKey(
        hKey,
        &myKeyStr,
        KEY_WRITE,
        REG_OPTION_NON_VOLATILE,
        NULL,
        WDF_NO_OBJECT_ATTRIBUTES,
        &subkey
    );
    if (!NT_SUCCESS(status)) {
        //MessageBox(NULL, L"失败终止", L"终止", 0);
        if (subkey) {
            WdfObjectDelete(subkey);
        }
        return;
    }

    if (dataValue == NULL) {
        //status = WdfRegistryAssignULong(subkey, &valueName, datasizeValue);
        status = WdfRegistryAssignValue(hKey, &valueName, REG_DWORD, 4, &datasizeValue);
        if (!NT_SUCCESS(status)) {
            return;
        }
    }
    else {
        status = WdfRegistryAssignValue(hKey, &valueName, REG_BINARY, datasizeValue, dataValue);
        if (!NT_SUCCESS(status)) {
            return;
        }
    }
    if (subkey) {
        WdfObjectDelete(subkey);
    }

    if (hKey) {
        WdfObjectDelete(hKey);
    }
}
