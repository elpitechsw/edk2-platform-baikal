/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/GpioLib.h>

#include <BS1000.h>

EFI_STATUS
EFIAPI
NonDiscoverableOhciEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  GpioOutSet(BS1000_GPIO32_BASE, 6);
  GpioDirSet(BS1000_GPIO32_BASE, 6);

  Status = RegisterNonDiscoverableMmioDevice (
             NonDiscoverableDeviceTypeOhci,
             NonDiscoverableDeviceDmaTypeCoherent,
             NULL,
             NULL,
             1,
             BS1000_OHCI_BASE,
             BS1000_OHCI_SIZE
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to register, Status: %r\n",
      __FUNCTION__,
      Status
      ));

    return Status;
  }

  return EFI_SUCCESS;
}
