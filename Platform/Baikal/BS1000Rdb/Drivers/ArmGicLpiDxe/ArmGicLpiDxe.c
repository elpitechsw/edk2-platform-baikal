/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ArmGicLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <BS1000.h>

#define ARM_GICD_TYPER               0x0004
#define ARM_GICD_TYPER_IDBITS_SHIFT  19
#define ARM_GICD_TYPER_IDBITS_MASK   (0x1F << ARM_GICD_TYPER_IDBITS_SHIFT)
#define ARM_GICR_CTLR                0x0000
#define ARM_GICR_CTLR_ENABLE_LPIS    BIT0
#define ARM_GICR_PROPBASER           0x0070
#define ARM_GICR_PENDBASER           0x0078
#define ARM_GICR_PENDBASER_PTZ       BIT62

EFI_STATUS
EFIAPI
ArmGicLpiDxeInitialize (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                 ChipIdx;
  UINTN                 Core;
  EFI_PHYSICAL_ADDRESS  GicDistributorBase;
  EFI_PHYSICAL_ADDRESS  GicRedistributorsBase;
  UINTN                 IdBits;
  EFI_PHYSICAL_ADDRESS  LpiTablesBase;
  UINTN                 LpiTablesSize;
  EFI_PHYSICAL_ADDRESS  PendTableAddr;
  UINT64                PropBaser;
  EFI_STATUS            Status;

  GicDistributorBase    = FixedPcdGet64 (PcdGicDistributorBase);
  GicRedistributorsBase = FixedPcdGet64 (PcdGicRedistributorsBase);

  LpiTablesBase = (EFI_PHYSICAL_ADDRESS) (BASE_4GB - 1);
  LpiTablesSize = SIZE_64KB * (1 + BS1000_CORE_COUNT * PLATFORM_CHIP_COUNT);

  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIReclaimMemory,
                  EFI_SIZE_TO_PAGES (LpiTablesSize),
                  &LpiTablesBase
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  ZeroMem ((VOID *) LpiTablesBase, LpiTablesSize);
  IdBits = (MmioRead32 (GicDistributorBase + ARM_GICD_TYPER) & ARM_GICD_TYPER_IDBITS_MASK) >>
           ARM_GICD_TYPER_IDBITS_SHIFT;

  PropBaser     = LpiTablesBase | IdBits;
  PendTableAddr = LpiTablesBase + SIZE_64KB;

  for (ChipIdx = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Core = 0; Core < BS1000_CORE_COUNT; ++Core) {
      EFI_PHYSICAL_ADDRESS  GicRdistBase =
        PLATFORM_ADDR_OUT_CHIP(ChipIdx, GicRedistributorsBase) + 0x20000 * Core;
      UINT64                PendBaser = ARM_GICR_PENDBASER_PTZ | PendTableAddr;

      MmioWrite64 (GicRdistBase + ARM_GICR_PROPBASER, PropBaser);
      MmioWrite64 (GicRdistBase + ARM_GICR_PENDBASER, PendBaser);
      MmioOr32 (GicRdistBase + ARM_GICR_CTLR, ARM_GICR_CTLR_ENABLE_LPIS);

      PendTableAddr += SIZE_64KB;
    }
  }

  return EFI_SUCCESS;
}
