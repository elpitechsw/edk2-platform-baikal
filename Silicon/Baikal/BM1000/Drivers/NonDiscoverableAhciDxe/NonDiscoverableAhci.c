/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>

#include <BM1000.h>

EFI_STATUS
EFIAPI
NonDiscoverableAhciEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN  Idx;
  CONST  EFI_PHYSICAL_ADDRESS  AhciBases[] = {BM1000_SATA0_BASE, BM1000_SATA1_BASE};
  CONST  UINTN                 AhciSizes[] = {BM1000_SATA0_SIZE, BM1000_SATA1_SIZE};

  STATIC_ASSERT (ARRAY_SIZE (AhciBases) == ARRAY_SIZE (AhciSizes));

  for (Idx = 0; Idx < ARRAY_SIZE (AhciBases); ++Idx) {
    EFI_STATUS  Status;
    Status = RegisterNonDiscoverableMmioDevice (
               NonDiscoverableDeviceTypeAhci,
               NonDiscoverableDeviceDmaTypeCoherent,
               NULL,
               NULL,
               1,
               AhciBases[Idx],
               AhciSizes[Idx]
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "%a: unable to register @ 0x%lx, Status: %r\n",
        __FUNCTION__, AhciBases[Idx], Status));

      return Status;
    }
  }

  return EFI_SUCCESS;
}
