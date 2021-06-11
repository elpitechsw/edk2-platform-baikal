/** @file
  Copyright (c) 2020, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/EmbeddedExternalDevice.h>

STATIC
EFI_STATUS
EFIAPI
NonDiscoverableDeviceXhciInitializer (
  IN  NON_DISCOVERABLE_DEVICE  *This
  )
{
  UINT32 * CONST  GsBusCfg0 = (UINT32 *)(This->Resources->AddrRangeMin + 0xC100);
  // Set AHB-prot/AXI-cache/OCP-ReqInfo for data/descriptor read/write
  *GsBusCfg0 = (*GsBusCfg0 & 0x0000FFFF) | 0xBB770000;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
PciEmulationXhciInit (
  VOID
  )
{
  UINTN  Idx;
  CONST EFI_PHYSICAL_ADDRESS  XhciBaseAddrs[] = {0x2C400000, 0x2C500000};

  for (Idx = 0; Idx < sizeof (XhciBaseAddrs) / sizeof (XhciBaseAddrs[0]); ++Idx) {
    EFI_STATUS  Status;
    Status = RegisterNonDiscoverableMmioDevice (
               NonDiscoverableDeviceTypeXhci,
               NonDiscoverableDeviceDmaTypeCoherent,
               NonDiscoverableDeviceXhciInitializer,
               NULL,
               1,
               XhciBaseAddrs[Idx],
               SIZE_1MB
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "%a: unable to register NonDiscoverableMmioDeviceTypeXhci @ 0x%lx, Status: %r\n",
        __FUNCTION__, XhciBaseAddrs[Idx], Status));

      return Status;
    }
  }

  return EFI_SUCCESS;
}

//
// Entry point
//
EFI_STATUS
EFIAPI
PciEmulationXhciEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  Status = PciEmulationXhciInit();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
