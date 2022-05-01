/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    driver and application

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_MouseLikeTouchPadFilter,
    0xbcc9d524,0x6c25,0x43b7,0xa2,0xbe,0x43,0x96,0x47,0xec,0xed,0x22);
// {bcc9d524-6c25-43b7-a2be-439647eced22}
