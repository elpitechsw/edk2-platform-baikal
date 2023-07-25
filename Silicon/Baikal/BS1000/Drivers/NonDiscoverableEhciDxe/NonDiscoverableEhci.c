/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/DwI2cLib.h>
#include <Library/GpioLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <BS1000.h>

#ifdef ELPITECH
STATIC CONST UINT8 UsbHubCfg0[] = {
  0, /* start offset */
  17, /* size of data */
  0x24, 0x04, 0x17, 0x25,
  0x00, 0x00, 0x9b, 0x20,
  0x00, 0x00, 0x00, 0x00,
  0x32, 0x32, 0x32, 0x32,
  0x32
};

STATIC CONST UINT8 UsbHubCfgff[] = {
  0xff, /* offset */
  1, /* size */
  0x01 /* enable */
};
#endif

EFI_STATUS
EFIAPI
NonDiscoverableEhciEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

#ifdef ELPITECH
  GpioOutRst(BS1000_GPIO32_BASE, 6);
  GpioDirSet(BS1000_GPIO32_BASE, 6);
  gBS->Stall(20);
  GpioOutSet(BS1000_GPIO32_BASE, 6);
  gBS->Stall(500);
  INTN Ret;
  Ret = I2cTxRx (BS1000_I2C6_BASE,
           100000000,
           0x2c,
           UsbHubCfg0,
           sizeof(UsbHubCfg0),
           0, 0);
  if (Ret < 0) {
    DEBUG((EFI_D_ERROR, "USB hub config: ret %d\n", Ret));
  }
  I2cTxRx (BS1000_I2C6_BASE,
           100000000,
           0x2c,
           UsbHubCfgff,
           sizeof(UsbHubCfgff),
           0, 0);
#endif

  Status = RegisterNonDiscoverableMmioDevice (
             NonDiscoverableDeviceTypeEhci,
             NonDiscoverableDeviceDmaTypeCoherent,
             NULL,
             NULL,
             1,
             BS1000_EHCI_BASE,
             BS1000_EHCI_SIZE
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: unable to register, Status: %r\n",
      __func__,
      Status
      ));

    return Status;
  }

  return EFI_SUCCESS;
}
