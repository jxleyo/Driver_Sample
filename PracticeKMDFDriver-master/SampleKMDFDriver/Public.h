/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_SampleKMDFDriver,
    0x768a896a,0x92f8,0x4616,0x91,0xad,0x4b,0x07,0x46,0x76,0x5b,0x46);
// {768a896a-92f8-4616-91ad-4b0746765b46}
