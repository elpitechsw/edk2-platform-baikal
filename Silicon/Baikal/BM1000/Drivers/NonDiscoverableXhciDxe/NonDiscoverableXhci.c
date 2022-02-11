/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>

#include <BM1000.h>

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

EFI_STATUS
EFIAPI
NonDiscoverableXhciEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN  Idx;
  CONST  EFI_PHYSICAL_ADDRESS  XhciBases[] = {BM1000_USB2_BASE, BM1000_USB3_BASE};
  CONST  UINTN                 XhciSizes[] = {BM1000_USB2_SIZE, BM1000_USB3_SIZE};

  STATIC_ASSERT (ARRAY_SIZE (XhciBases) == ARRAY_SIZE (XhciSizes));

  for (Idx = 0; Idx < ARRAY_SIZE (XhciBases); ++Idx) {
    EFI_STATUS  Status;
    Status = RegisterNonDiscoverableMmioDevice (
               NonDiscoverableDeviceTypeXhci,
               NonDiscoverableDeviceDmaTypeCoherent,
               NonDiscoverableDeviceXhciInitializer,
               NULL,
               1,
               XhciBases[Idx],
               XhciSizes[Idx]
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "%a: unable to register @ 0x%lx, Status: %r\n",
        __FUNCTION__, XhciBases[Idx], Status));

      return Status;
    }
  }

  return EFI_SUCCESS;
}
