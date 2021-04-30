/** @file
  Common driver for devices connected to the I2C bus
  of the Baikal-M-based Mini-ITX board.

  Copyright 2020 Baikal Electronics

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <libfdt.h>

#include <Guid/Fdt.h>
#include <Guid/FdtHob.h>

#include <Protocol/NonDiscoverableDevice.h>
#include <Protocol/I2cBusConfigurationManagement.h>
#include <Protocol/I2cEnumerate.h>
#include <Protocol/FdtClient.h>

#define I2C_DEVICES_BUS_FREQUENCY_DEFAULT   400000
#define I2C_DEVICES_HOST_COMPAT_STRING      "snps,designware-i2c"

typedef enum {
  I2C_DEVICES_BUS_CONFIG_DEFAULT,
  I2C_DEVICES_BUS_CONFIG_NUM
} I2C_DEVICES_BUS_CONFIG_ID;

typedef enum {
  I2C_DEVICES_DRIVER_NUM
} I2C_DEVICES_DRIVER_ID;

typedef struct {
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR StartDesc;
  UINT8 EndDesc;
} ADDRESS_SPACE_DESCRIPTOR;

typedef struct I2C_DEVICES_DESC_S {
  struct I2C_DEVICES_DESC_S *Next;
  UINT32                    Address;
  EFI_I2C_DEVICE            I2cDevice;
} I2C_DEVICES_DESC;

typedef struct {
  UINTN                     BaseAddress;
  UINTN                     Size;
} I2C_DEVICES_HOST_REG_DESC;

typedef struct {
  UINTN                                         Signature;
  EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL ConfigManagement;
  EFI_I2C_ENUMERATE_PROTOCOL                    Enumerate;
  ADDRESS_SPACE_DESCRIPTOR                      MmioDesc;
  UINTN                                         ActiveConfiguration;
  UINTN                                         BusFrequency[I2C_DEVICES_BUS_CONFIG_NUM];
  I2C_DEVICES_DESC                              *Devices;
} I2C_DEVICES_HOST_DATA;

typedef struct I2C_DEVICES_HOST_DESC_S {
  UINTN                                         Signature;
  struct I2C_DEVICES_HOST_DESC_S                *Next;
  I2C_DEVICES_HOST_DATA                         Data;
} I2C_DEVICES_HOST_DESC;

typedef struct {
  CONST CHAR8                                   *CompatibleString;
  UINT32                                        BusConfiguration;
  UINT32                                        HardwareRevision;
  CONST EFI_GUID                                *Guid;
} I2C_DEVICES_DRIVER_DATA;

#define I2C_DEVICES_HOST_DATA_SIGNATURE         SIGNATURE_32 ('i', '2', 'c', 'd')
#define I2C_DEVICES_HOST_DESC_SIGNATURE         SIGNATURE_32 ('i', '2', 'c', 'x')
#define I2C_DEVICES_HOST_DATA_FROM_CM(a)        CR (a, I2C_DEVICES_HOST_DATA, ConfigManagement, I2C_DEVICES_HOST_DATA_SIGNATURE)
#define I2C_DEVICES_HOST_DATA_FROM_ENUMERATE(a) CR (a, I2C_DEVICES_HOST_DATA,        Enumerate, I2C_DEVICES_HOST_DATA_SIGNATURE)
#define I2C_DEVICES_HOST_DESC_FROM_DATA(a)      CR (a, I2C_DEVICES_HOST_DESC,             Data, I2C_DEVICES_HOST_DESC_SIGNATURE)

STATIC CONST I2C_DEVICES_DRIVER_DATA mI2cDrivers[] = {
/* ID                            CompatibleString                BusConfiguration  HardwareRevision                        Guid */
};

STATIC
EFI_STATUS
EFIAPI 
I2cDevicesEnableBusConfiguration (
  IN CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *This,
  IN UINTN                                               I2cBusConfiguration,
  IN EFI_EVENT                                           Event      OPTIONAL,
  IN EFI_STATUS                                          *I2cStatus OPTIONAL
  )
{
  EFI_STATUS                          Status;
  I2C_DEVICES_HOST_DATA               *Data;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Data = I2C_DEVICES_HOST_DATA_FROM_CM (This);

  if (I2cBusConfiguration < I2C_DEVICES_BUS_CONFIG_NUM) {
    if (I2cBusConfiguration != Data->ActiveConfiguration) {
      Data->ActiveConfiguration = I2cBusConfiguration;
    }
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NO_MAPPING;
  }

  if (I2cStatus != NULL) {
    *I2cStatus = Status;
  }
  if (Event != NULL) {
    gBS->SignalEvent (Event);
  }

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
I2cDevicesEnumerate (
  IN CONST EFI_I2C_ENUMERATE_PROTOCOL *This,
  IN OUT CONST EFI_I2C_DEVICE         **Device
  )
{
  EFI_STATUS                          Status;
  I2C_DEVICES_HOST_DATA               *Data;
  I2C_DEVICES_DESC                    *Desc;

  if ((This == NULL) || (Device == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Data = I2C_DEVICES_HOST_DATA_FROM_ENUMERATE (This);

  if (*Device == NULL) {
    *Device = (Data->Devices == NULL) ?
              NULL :
              &Data->Devices->I2cDevice;
    Status = EFI_SUCCESS;
  } else {
    if (Data->Devices == NULL) {
      Status = EFI_NO_MAPPING;
    } else {
      Desc = Data->Devices;
      while ((Desc != NULL) && (&Desc->I2cDevice != *Device)) {
        Desc = Desc->Next;
      }
      if (Desc != NULL) {
        *Device = (Desc->Next == NULL) ?
                  NULL :
                  &Desc->Next->I2cDevice;
        Status = EFI_SUCCESS;
      } else {
        Status = EFI_NO_MAPPING;
      }
    }
  }

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
I2cDevicesGetBusFrequency (
  IN CONST EFI_I2C_ENUMERATE_PROTOCOL *This,
  IN UINTN                            I2cBusConfiguration,
  OUT UINTN                           *BusClockHertz
  )
{
  EFI_STATUS                          Status;
  I2C_DEVICES_HOST_DATA               *Data;

  if ((This == NULL) || (BusClockHertz == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Data = I2C_DEVICES_HOST_DATA_FROM_ENUMERATE(This);

  if (I2cBusConfiguration < I2C_DEVICES_BUS_CONFIG_NUM) {
    *BusClockHertz = Data->BusFrequency[I2cBusConfiguration];
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NO_MAPPING;
  }

  return Status;
}

STATIC
EFI_STATUS
RegisterDevice (
  IN  EFI_GUID                        *TypeGuid,
  IN  ADDRESS_SPACE_DESCRIPTOR        *Desc,
  OUT EFI_HANDLE                      *Handle
  )
{
  NON_DISCOVERABLE_DEVICE             *Device;
  EFI_STATUS                          Status;

  Device = (NON_DISCOVERABLE_DEVICE *)AllocateZeroPool (sizeof (*Device));
  if (Device == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Device->Type = TypeGuid;
  Device->DmaType = NonDiscoverableDeviceDmaTypeNonCoherent;
  Device->Resources = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Desc;

  Status = gBS->InstallMultipleProtocolInterfaces (Handle,
                                                   &gEdkiiNonDiscoverableDeviceProtocolGuid,
                                                   Device, NULL);
  if (EFI_ERROR (Status)) {
    goto FreeDevice;
  }
  return EFI_SUCCESS;

FreeDevice:
  FreePool (Device);

  return Status;
}

STATIC
UINTN
FindDriverMatch (
  IN CONST CHAR8                   *CompatibleString
  )
{
  UINTN                            Index;
  
  for (Index = 0;
       (Index < I2C_DEVICES_DRIVER_NUM) &&
       (AsciiStrCmp (CompatibleString,
                     mI2cDrivers[Index].CompatibleString) != 0);
       Index++);
  return Index;
}

STATIC
VOID
FillI2cDeviceDesc (
  IN  UINTN                            DriverIndex,
  IN  UINT32                           DeviceIndex,
  IN  CONST UINT32                     *DeviceAddressPtr,
  OUT EFI_I2C_DEVICE                   *I2cDevice
  )
{
  I2cDevice->DeviceGuid = mI2cDrivers[DriverIndex].Guid;
  I2cDevice->DeviceIndex = DeviceIndex;
  I2cDevice->HardwareRevision = mI2cDrivers[DriverIndex].HardwareRevision;
  I2cDevice->I2cBusConfiguration = mI2cDrivers[DriverIndex].BusConfiguration;
  I2cDevice->SlaveAddressCount = 1;
  I2cDevice->SlaveAddressArray = DeviceAddressPtr;
}

STATIC
VOID
AddDeviceDesc (
  IN UINTN                         DriverIndex,
  IN UINT32                        DeviceAddress,
  IN OUT I2C_DEVICES_DESC          **Desc
  )
{
  UINT32                           DeviceIndex;
  I2C_DEVICES_DESC                 *DeviceDesc;
  I2C_DEVICES_DESC                 DummyDeviceDesc;

  DummyDeviceDesc.Next = NULL;
  if (Desc != NULL) {
    DeviceDesc = (*Desc == NULL) ? &DummyDeviceDesc : *Desc;
    DeviceIndex = 0;
    while (DeviceDesc->Next != NULL) {
      DeviceDesc = DeviceDesc->Next;
      DeviceIndex++;
    }
    DeviceDesc->Next = AllocateZeroPool (sizeof (I2C_DEVICES_DESC));
    if (DeviceDesc->Next != NULL) {
      DeviceDesc->Next->Next = NULL;
      DeviceDesc->Next->Address = DeviceAddress;
      FillI2cDeviceDesc (DriverIndex,
                         (*Desc == NULL) ? 0 : DeviceIndex + 1,
                         &DeviceDesc->Next->Address,
                         &DeviceDesc->Next->I2cDevice);
      if (*Desc == NULL) {
        *Desc = DeviceDesc->Next;
      }
    }
  }
}

STATIC
VOID *
GetDeviceTreeBase (
  VOID
  )
{
  VOID              *Hob;
  VOID              *DeviceTreeBase;

  DeviceTreeBase = NULL;
  Hob = GetFirstGuidHob (&gFdtHobGuid);
  if ((Hob != NULL) &
      (GET_GUID_HOB_DATA_SIZE (Hob) == sizeof (UINT64))) {
    DeviceTreeBase = (VOID *)(UINTN)*(UINT64 *)GET_GUID_HOB_DATA (Hob);
    if (fdt_check_header (DeviceTreeBase) != 0) {
      DEBUG ((EFI_D_ERROR, "%a: No DTB found @ 0x%p\n", __FUNCTION__,
              DeviceTreeBase));
      DeviceTreeBase = NULL;
    }
  }
  return DeviceTreeBase;
}

STATIC
EFI_STATUS
I2cDevicesGetDeviceAddress (
  IN  FDT_CLIENT_PROTOCOL          *FdtClient,
  IN  INT32                        Node,
  OUT UINT32                       *DeviceAddress
  )
{
  EFI_STATUS                       Status;
  UINT32                           PropertySize;
  CONST UINT32                     *Property;

  Status = FdtClient->GetNodeProperty (FdtClient, Node, "reg",
                                       (CONST VOID **)&Property,
                                       &PropertySize);
  if (!EFI_ERROR (Status)) {
    if (PropertySize == sizeof (UINT32)) {
      *DeviceAddress = SwapBytes32 (Property[0]);
    } else {
      Status = EFI_NO_MAPPING;
    }
  }
  return Status;
}

STATIC
UINTN
I2cDevicesFindDeviceDriver (
  IN FDT_CLIENT_PROTOCOL           *FdtClient,
  IN INT32                         Node
  )
{
  EFI_STATUS                       Status;
  UINTN                            DriverIndex;
  UINT32                           PropertySize;
  CONST CHAR8                      *Property;
  CONST CHAR8                      *CompatibleString;

  DriverIndex = I2C_DEVICES_DRIVER_NUM;
  Status = FdtClient->GetNodeProperty (FdtClient, Node, "compatible",
                                       (CONST VOID **)&Property,
                                       &PropertySize);
  if (!EFI_ERROR (Status)) {
    for (CompatibleString = Property;
         (CompatibleString < (Property + PropertySize)) &&
         (*CompatibleString != 0);
         CompatibleString += AsciiStrLen (CompatibleString) + 1) {
      DriverIndex = FindDriverMatch (CompatibleString);
      if (DriverIndex < I2C_DEVICES_DRIVER_NUM) {
        break;
      }
    }
  }
  return DriverIndex;
}

STATIC
VOID
I2cDevicesPopulateDevicesDesc (
  IN FDT_CLIENT_PROTOCOL           *FdtClient,
  IN INT32                         Node,
  IN OUT I2C_DEVICES_DESC          **Desc
  )
{
  EFI_STATUS                       Status;
  INT32                            DevNode;
  UINT32                           DeviceAddress;
  UINTN                            DriverIndex;
  VOID                             *DeviceTreeBase;

  DeviceTreeBase = GetDeviceTreeBase ();
  if (DeviceTreeBase != NULL) {
    for (DevNode = fdt_first_subnode(DeviceTreeBase, Node);
	     DevNode >= 0;
	     DevNode = fdt_next_subnode(DeviceTreeBase, DevNode)) {
      if (DevNode >= 0) {
        DriverIndex = I2cDevicesFindDeviceDriver (FdtClient, DevNode);
        if (DriverIndex < I2C_DEVICES_DRIVER_NUM) {
          Status = I2cDevicesGetDeviceAddress (FdtClient, DevNode,
                                               &DeviceAddress);
          if (!EFI_ERROR (Status)) {
            AddDeviceDesc (DriverIndex, DeviceAddress, Desc);
          }
        }
      }
    }
  }
}

STATIC
VOID
I2cDevicesPurgeDevicesDesc (
  IN I2C_DEVICES_DESC              *Desc
  )
{
  I2C_DEVICES_DESC                 *Next;

  Next = Desc;
  while (Next != NULL) {
    Desc = Next;
    Next = Desc->Next;
    FreePool (Desc);
  }
}

STATIC
EFI_STATUS
I2cDevicesCheckNodeStatus (
  IN  FDT_CLIENT_PROTOCOL          *FdtClient,
  IN  INT32                        Node
  )
{
  EFI_STATUS                       Status;
  UINT32                           PropertySize;
  CONST CHAR8                      *Property;

  Status = FdtClient->GetNodeProperty (FdtClient, Node, "status",
                                       (CONST VOID **)&Property,
                                       &PropertySize);
  if (!EFI_ERROR (Status)) {
    if (AsciiStrCmp (Property, "okay") != 0) {
      Status = EFI_DEVICE_ERROR;
    }
  }
  return Status;
}

STATIC
EFI_STATUS
I2cDevicesGetMmioRegion (
  IN  FDT_CLIENT_PROTOCOL          *FdtClient,
  IN  INT32                        Node,
  OUT I2C_DEVICES_HOST_REG_DESC    *MmioRegion
  )
{
  EFI_STATUS                       Status;
  UINT32                           PropertySize;
  CONST UINT32                     *Property;

  Status = FdtClient->GetNodeProperty (FdtClient, Node, "reg",
                                       (CONST VOID **)&Property,
                                       &PropertySize);
  if (!EFI_ERROR (Status)) {
    if (PropertySize == (2 * sizeof (UINT64))) {
      MmioRegion->BaseAddress = SwapBytes64 (((UINT64 *)Property)[0]);
      MmioRegion->Size = SwapBytes64 (((UINT64 *)Property)[1]);
    } else {
      Status = EFI_NO_MAPPING;
    }
  }
  return Status;
}

STATIC
EFI_STATUS
I2cDevicesGetClockFrequency (
  IN  FDT_CLIENT_PROTOCOL          *FdtClient,
  IN  INT32                        Node,
  OUT UINT32                       *ClockFrequency
  )
{
  EFI_STATUS                       Status;
  UINT32                           PropertySize;
  CONST UINT32                     *Property;

  Status = FdtClient->GetNodeProperty (FdtClient, Node, "clock-frequency",
                                       (CONST VOID **)&Property,
                                       &PropertySize);
  if (!EFI_ERROR (Status)) {
    if (PropertySize >= sizeof (UINT32)) {
      *ClockFrequency = SwapBytes32 (Property[0]);
    } else {
      Status = EFI_NO_MAPPING;
    }
  }
  return Status;
}

STATIC
VOID
I2cDevicesFillMmioRegionDesc (
  IN  UINTN                     BaseAddress,
  IN  UINTN                     Size,
  OUT ADDRESS_SPACE_DESCRIPTOR  *Desc
  )
{
  Desc->StartDesc.Desc = ACPI_ADDRESS_SPACE_DESCRIPTOR;
  Desc->StartDesc.Len = sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) - 3;
  Desc->StartDesc.ResType = ACPI_ADDRESS_SPACE_TYPE_MEM;
  Desc->StartDesc.GenFlag = 0;
  Desc->StartDesc.SpecificFlag = 0;
  Desc->StartDesc.AddrLen = Size;
  Desc->StartDesc.AddrRangeMin = BaseAddress;
  Desc->StartDesc.AddrRangeMax = BaseAddress + Size - 1;
  Desc->StartDesc.AddrSpaceGranularity = (BaseAddress > MAX_UINT32) ? 64 : 32;
  Desc->StartDesc.AddrTranslationOffset = 0;
  Desc->EndDesc = ACPI_END_TAG_DESCRIPTOR;
}

STATIC
VOID
I2cDevicesFillHostData (
  IN  FDT_CLIENT_PROTOCOL          *FdtClient,
  IN  INT32                        Node,
  OUT I2C_DEVICES_HOST_DATA        *Data
  )
{
  EFI_STATUS                       Status;
  UINTN                            Index;
  UINT32                           DefaultBusFrequency;

  Data->Signature = I2C_DEVICES_HOST_DATA_SIGNATURE;
  Data->ConfigManagement.EnableI2cBusConfiguration = I2cDevicesEnableBusConfiguration;
  Data->Enumerate.Enumerate = I2cDevicesEnumerate;
  Data->Enumerate.GetBusFrequency = I2cDevicesGetBusFrequency;
  Data->ActiveConfiguration = 0;
  Status = I2cDevicesGetClockFrequency (FdtClient, Node, &DefaultBusFrequency);
  if (EFI_ERROR (Status)) {
    DefaultBusFrequency = I2C_DEVICES_BUS_FREQUENCY_DEFAULT;
  }
  for (Index = 0; Index < I2C_DEVICES_BUS_CONFIG_NUM; Index++) {
    Data->BusFrequency[Index] = DefaultBusFrequency;
  }
  Data->Devices = NULL;
  I2cDevicesPopulateDevicesDesc (FdtClient, Node, &Data->Devices);
}

STATIC
VOID
I2cDevicesAddHostDesc (
  IN FDT_CLIENT_PROTOCOL           *FdtClient,
  IN INT32                         Node,
  IN I2C_DEVICES_HOST_REG_DESC     *MmioRegion,
  IN OUT I2C_DEVICES_HOST_DESC     **Desc
)
{
  UINTN                            HostIndex;
  I2C_DEVICES_HOST_DESC            *HostDesc;
  I2C_DEVICES_HOST_DESC            DummyHostDesc;

  DummyHostDesc.Next = NULL;
  if (Desc != NULL) {
    HostDesc = (*Desc == NULL) ? &DummyHostDesc : *Desc;
    HostIndex = 0;
    while (HostDesc->Next != NULL) {
      HostDesc = HostDesc->Next;
      HostIndex++;
    }
    HostDesc->Next = AllocateZeroPool (sizeof (I2C_DEVICES_HOST_DESC));
    if (HostDesc->Next != NULL) {
      HostDesc->Next->Signature = I2C_DEVICES_HOST_DESC_SIGNATURE;
      HostDesc->Next->Next = NULL;
      I2cDevicesFillHostData (FdtClient, Node, &HostDesc->Next->Data);
      if (HostDesc->Next->Data.Devices != NULL) {
        I2cDevicesFillMmioRegionDesc (MmioRegion->BaseAddress, MmioRegion->Size,
                                      &HostDesc->Next->Data.MmioDesc);
        if (*Desc == NULL) {
          *Desc = HostDesc->Next;
        }
      } else {
        FreePool (HostDesc->Next);
        HostDesc->Next = NULL;
      }
    }
  }
}

STATIC
VOID
I2cDevicesParseFdt (
  IN OUT I2C_DEVICES_HOST_DESC     **Desc
  )
{
  EFI_STATUS                       Status;
  INT32                            Node;
  I2C_DEVICES_HOST_REG_DESC        MmioRegion;
  FDT_CLIENT_PROTOCOL              *FdtClient;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid,
                                NULL,
                                (VOID **)&FdtClient);
  Node = 0;
  while (!EFI_ERROR (Status)) {
    Status = FdtClient->FindNextCompatibleNode (FdtClient,
                                                I2C_DEVICES_HOST_COMPAT_STRING,
                                                Node, &Node);
    if (!EFI_ERROR (Status)) {
      Status = I2cDevicesCheckNodeStatus (FdtClient, Node);
      if (!EFI_ERROR (Status)) {
        Status = I2cDevicesGetMmioRegion (FdtClient, Node, &MmioRegion);
        if (!EFI_ERROR (Status)) {
          I2cDevicesAddHostDesc (FdtClient, Node, &MmioRegion, Desc);
        }
      }
      Status = EFI_SUCCESS;
    }
  }
}

EFI_STATUS
EFIAPI
I2cDevicesDxeEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      Handle;
  I2C_DEVICES_HOST_DESC           *I2cHost;

  I2cHost = NULL;
  I2cDevicesParseFdt (&I2cHost);
  while (I2cHost != NULL) {
    Handle = NULL;
    Status = RegisterDevice (&gBaikalNonDiscoverableI2cMasterGuid,
                             &I2cHost->Data.MmioDesc, &Handle);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = gBS->InstallMultipleProtocolInterfaces (&Handle,
                                          &gEfiI2cBusConfigurationManagementProtocolGuid,
                                          &I2cHost->Data.ConfigManagement,
                                          &gEfiI2cEnumerateProtocolGuid,
                                          &I2cHost->Data.Enumerate,
                                          NULL);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    I2cHost = I2cHost->Next;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
I2cDevicesDxeUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                           HandleCount;
  UINTN                           Index;
  NON_DISCOVERABLE_DEVICE         *Device;
  EFI_I2C_ENUMERATE_PROTOCOL      *I2cEnumerate;
  I2C_DEVICES_HOST_DESC           *I2cHost;


  //
  // Retrieve all non-discoverable I/O handles in the handle database
  //
  Status = gBS->LocateHandleBuffer (ByProtocol,
                                    &gEdkiiNonDiscoverableDeviceProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Disconnect the driver from the handles in the handle database
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->OpenProtocol (
                    HandleBuffer[Index],
                    &gEdkiiNonDiscoverableDeviceProtocolGuid,
                    (VOID **) &Device,
                    gImageHandle,
                    HandleBuffer[Index],
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status) &&
        CompareGuid (Device->Type, &gBaikalNonDiscoverableI2cMasterGuid)) {
      Status = gBS->OpenProtocol (
                      HandleBuffer[Index],
                      &gEfiI2cEnumerateProtocolGuid,
                      (VOID **) &I2cEnumerate,
                      gImageHandle,
                      HandleBuffer[Index],
                      EFI_OPEN_PROTOCOL_GET_PROTOCOL
                      );
      if (!EFI_ERROR (Status)) {
        I2cHost = I2C_DEVICES_HOST_DESC_FROM_DATA (I2C_DEVICES_HOST_DATA_FROM_ENUMERATE (I2cEnumerate));
        Status = gBS->UninstallMultipleProtocolInterfaces (HandleBuffer + Index,
                                              &gEfiI2cBusConfigurationManagementProtocolGuid,
                                              I2cHost->Data.ConfigManagement,
                                              &gEfiI2cEnumerateProtocolGuid,
                                              I2cHost->Data.Enumerate,
                                              &gEdkiiNonDiscoverableDeviceProtocolGuid,
                                              Device,
                                              NULL);
        if (EFI_ERROR (Status)) {
          return Status;
        }
        if (I2cHost->Data.Devices != NULL) {
          I2cDevicesPurgeDevicesDesc (I2cHost->Data.Devices);
          I2cHost->Data.Devices = NULL;
        }
        FreePool (I2cHost);
        FreePool (Device);
      }
    }
  }

  //
  // Free the handle array
  //
  gBS->FreePool (HandleBuffer);

  return Status;
}
