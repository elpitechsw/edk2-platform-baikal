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

#define CTLR_ENABLE_LPIS_BIT  BIT0
#define PENDBASER_PTZ_BIT     BIT62

EFI_STATUS
EFIAPI
ArmGicLpiDxeInitialize (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN                 Core;
  EFI_PHYSICAL_ADDRESS  GicDistributorBase;
  EFI_PHYSICAL_ADDRESS  GicRedistributorsBase;
  EFI_PHYSICAL_ADDRESS  LpiTablesBase;
  UINTN                 LpiTablesSize;
  EFI_PHYSICAL_ADDRESS  PendTableAddr[48];
  EFI_PHYSICAL_ADDRESS  PropTableAddr;
  EFI_STATUS            Status;

  GicDistributorBase    = FixedPcdGet64 (PcdGicDistributorBase);
  GicRedistributorsBase = FixedPcdGet64 (PcdGicRedistributorsBase);

  LpiTablesBase = (EFI_PHYSICAL_ADDRESS) (BASE_4GB - 1);
  LpiTablesSize = 64 * 1024 * (48 + 16 * 2 + 4);

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

  PropTableAddr    = LpiTablesBase;
  PendTableAddr[0] = LpiTablesBase + 64 * 1024;

  for (Core = 0; Core < 48; ++Core) {
    EFI_PHYSICAL_ADDRESS  GicRdistBase = GicRedistributorsBase + 0x20000 * Core;
    UINT8                 IdBits = (MmioRead32 (GicDistributorBase + 0x04) >> 19) & 0x1F;
    UINT64                PendBaser;
    UINT64                PropBaser = PropTableAddr | IdBits;

    PendTableAddr[Core] = PendTableAddr[0] + 64 * 1024 * Core;
    PendBaser = PENDBASER_PTZ_BIT | PendTableAddr[Core];
    MmioWrite64 (GicRdistBase + 0x70, PropBaser);
    MmioWrite64 (GicRdistBase + 0x78, PendBaser);
    MmioWrite32 (GicRdistBase + 0x00, MmioRead32 (GicRdistBase + 0x00) | CTLR_ENABLE_LPIS_BIT);

    while (MmioRead64 (GicRdistBase + 0x00) & 1);
  }

  return EFI_SUCCESS;
}
