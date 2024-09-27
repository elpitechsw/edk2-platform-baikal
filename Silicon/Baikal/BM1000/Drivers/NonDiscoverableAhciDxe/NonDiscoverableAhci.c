/** @file
  Copyright (c) 2020 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FdtClient.h>

#define SATA_CAP       0x00
#define SATA_CAP_SSS   BIT27
#define SATA_CAP_SMPS  BIT28

#define SATA_GHC       0x04
#define SATA_GHC_HR    BIT0

#define SATA_PI        0x0C

STATIC
EFI_STATUS
EFIAPI
NonDiscoverableDeviceAhciInitializer (
  IN  NON_DISCOVERABLE_DEVICE  *This
  )
{
  UINT64  Timestamp;

  // Reset HBA
  MmioOr32 (
    This->Resources->AddrRangeMin +
      SATA_GHC,
      SATA_GHC_HR
    );

  Timestamp = GetPerformanceCounter ();
  while (MmioRead32 (This->Resources->AddrRangeMin + SATA_GHC) & SATA_GHC_HR) {
    if (GetTimeInNanoSecond (GetPerformanceCounter () - Timestamp) > 100000) {
      DEBUG ((
        EFI_D_ERROR,
        "%a(0x%lx): SATA_GHC.HR timeout\n",
        __func__,
        This->Resources->AddrRangeMin
        ));
      break;
    }
  }

  MmioAndThenOr32 (
    This->Resources->AddrRangeMin +
      SATA_CAP,
     ~SATA_CAP_SSS, // Disable staggered spin-up support
      SATA_CAP_SMPS // Enable mechanical presence switch support
    );

  // 1 Port implemented
  MmioWrite32 (This->Resources->AddrRangeMin + SATA_PI, 1);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
NonDiscoverableAhciEntryPoint (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  FDT_CLIENT_PROTOCOL  *FdtClient;
  INT32                 Node = 0;
  CONST VOID           *Prop;
  UINT32                PropSize;
  EFI_STATUS            Status;

  Status = gBS->LocateProtocol (&gFdtClientProtocolGuid, NULL, (VOID **) &FdtClient);
  ASSERT_EFI_ERROR (Status);

  while (TRUE) {
    Status = FdtClient->FindNextCompatibleNode (FdtClient, "baikal,bm1000-ahci", Node, &Node);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (!FdtClient->IsNodeEnabled (FdtClient, Node)) {
      continue;
    }

    if (FdtClient->GetNodeProperty (FdtClient, Node, "reg", &Prop, &PropSize) == EFI_SUCCESS &&
        PropSize == 16) {
      CONST EFI_PHYSICAL_ADDRESS  AhciBase = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + 0));
      CONST UINTN                 AhciSize = SwapBytes64 (ReadUnaligned64 ((CONST UINT64 *) Prop + 1));

      Status = RegisterNonDiscoverableMmioDevice (
                 NonDiscoverableDeviceTypeAhci,
                 NonDiscoverableDeviceDmaTypeCoherent,
                 NonDiscoverableDeviceAhciInitializer,
                 NULL,
                 1,
                 AhciBase,
                 AhciSize
                 );

      if (EFI_ERROR (Status)) {
        DEBUG ((
          EFI_D_ERROR,
          "%a: unable to register @ 0x%lx, Status: %r\n",
          __func__,
          AhciBase,
          Status
          ));
      }
    }
  }

  return EFI_SUCCESS;
}
