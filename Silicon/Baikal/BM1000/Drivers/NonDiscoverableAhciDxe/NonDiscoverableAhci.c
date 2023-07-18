/** @file
  Copyright (c) 2020 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/TimerLib.h>

#include <BM1000.h>

#define SATA_CAP                  0x00
#define SATA_CAP_SSS              BIT27
#define SATA_CAP_SMPS             BIT28

#define SATA_GHC                  0x04
#define SATA_GHC_HR               BIT0

#define SATA_PI                   0x0C

#define SATA_P0_SSTS              0x128
#define SATA_P_SSTS_DET           0x1
#define SATA_P_SSTS_DET_PCE       0x3
#define SATA_P_SSTS_DET_MASK      0xF

#define SATA_P0_SCTL              0x12C
#define SATA_P_SCTL_DET_INIT      0x1
#define SATA_P_SCTL_DET_MASK      0x7
#define SATA_P_SCTL_SPD_1_5_GBPS  (0x1 << 4)

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
  // Limit speed negotiation to SATA 1.5 Gb/s (What for?)
  MmioWrite32 (
    This->Resources->AddrRangeMin +
      SATA_P0_SCTL,
      SATA_P_SCTL_DET_INIT |
      SATA_P_SCTL_SPD_1_5_GBPS
    );

  // Keep DET set at least for 1ms to ensure that at least one COMRESET signal is sent
  MicroSecondDelay (1000);
  MmioAnd32 (
    This->Resources->AddrRangeMin +
      SATA_P0_SCTL,
     ~SATA_P_SCTL_DET_MASK
    );

  Timestamp = GetPerformanceCounter ();
  while (TRUE) {
    CONST UINT32  Ssts = MmioRead32 (This->Resources->AddrRangeMin + SATA_P0_SSTS);

    if ((Ssts & SATA_P_SSTS_DET_MASK) == SATA_P_SSTS_DET ||
        (Ssts & SATA_P_SSTS_DET_MASK) == SATA_P_SSTS_DET_PCE) {
      break;
    }

    if (GetTimeInNanoSecond (GetPerformanceCounter () - Timestamp) > 1000000) {
      break;
    }
  }

  return EFI_SUCCESS;
}

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
               NonDiscoverableDeviceAhciInitializer,
               NULL,
               1,
               AhciBases[Idx],
               AhciSizes[Idx]
               );

    if (EFI_ERROR (Status)) {
      DEBUG ((
        EFI_D_ERROR,
        "%a: unable to register @ 0x%lx, Status: %r\n",
        __func__,
        AhciBases[Idx],
        Status
        ));
      return Status;
    }
  }

  return EFI_SUCCESS;
}
