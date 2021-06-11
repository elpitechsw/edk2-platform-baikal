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
PciEmulationAhciInit (
  VOID
  )
{
  UINTN  Idx;
  CONST EFI_PHYSICAL_ADDRESS  AhciBaseAddrs[] = {0x2C600000, 0x2C610000};

  for (Idx = 0; Idx < sizeof (AhciBaseAddrs) / sizeof (AhciBaseAddrs[0]); ++Idx) {
    EFI_STATUS  Status;
    Status = RegisterNonDiscoverableMmioDevice (
               NonDiscoverableDeviceTypeAhci,
               NonDiscoverableDeviceDmaTypeCoherent,
               NULL,
               NULL,
               1,
               AhciBaseAddrs[Idx],
               SIZE_4KB
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "%a: unable to register NonDiscoverableMmioDeviceTypeAhci @ 0x%lx, Status: %r\n",
        __FUNCTION__, AhciBaseAddrs[Idx], Status));

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
PciEmulationAhciEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  Status = PciEmulationAhciInit();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
