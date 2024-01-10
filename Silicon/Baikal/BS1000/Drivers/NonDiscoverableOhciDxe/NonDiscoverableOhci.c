/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>

#include <BS1000.h>

EFI_STATUS
EFIAPI
NonDiscoverableOhciEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN       ChipIdx;
  EFI_STATUS  Status;

  for (ChipIdx = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    Status = RegisterNonDiscoverableMmioDevice (
               NonDiscoverableDeviceTypeOhci,
               NonDiscoverableDeviceDmaTypeCoherent,
               NULL,
               NULL,
               1,
               PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_OHCI_BASE),
               BS1000_OHCI_SIZE
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((
        EFI_D_ERROR,
        "%a: unable to register 0x%llx, Status: %r\n",
        __func__,
        PLATFORM_ADDR_OUT_CHIP(ChipIdx, BS1000_OHCI_BASE),
        Status
        ));
    }
  }

  return EFI_SUCCESS;
}
