/** @file
  Copyright (c) 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/ArmLib.h>
#include <Library/BaikalMemoryRangeLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Protocol/FdtClient.h>
#include "AcpiPlatform.h"

#include <BS1000.h>

#if PLATFORM_CHIP_COUNT > 1

STATIC EFI_ACPI_6_4_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER SratHeaderTemplate = {
  BAIKAL_ACPI_HEADER (
    EFI_ACPI_6_4_SYSTEM_RESOURCE_AFFINITY_TABLE_SIGNATURE,
    0,
    EFI_ACPI_6_4_SYSTEM_RESOURCE_AFFINITY_TABLE_REVISION,
    0x54415253
    ),
  /* UINT32                         Reserved1 */
  1,
  /* UINT64                         Reserved2 */
  0
};

typedef struct {
  UINT64  Addr;
  UINT64  Size;
} MEMORY_REGION_DESCRIPTION;

STATIC
EFI_STATUS
GetMemoryRegions (
  OUT MEMORY_REGION_DESCRIPTION ** CONST  Regions,
  OUT UINTN * CONST                       Count
  )
{
  UINTN                       Amount;
  UINTN                       AddressCells;
  UINTN                       Idx;
  INT32                       Node = 0;
  MEMORY_REGION_DESCRIPTION  *MemReg = NULL;
  CONST UINT32               *Reg;
  UINTN                       RegCount = 0;
  UINTN                       SizeCells;
  EFI_STATUS                  Status;

  if (Regions == NULL || Count == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  while ((Status = GetMemoryRanges (&Node, &Reg, &Amount, &AddressCells, &SizeCells)) != EFI_NOT_FOUND) {
    if (EFI_ERROR (Status)) {
      if (MemReg != NULL) {
        FreePool (MemReg);
      }
      return Status;
    }

    MEMORY_REGION_DESCRIPTION  *TmpArray = (MEMORY_REGION_DESCRIPTION *) ReallocatePool (
                                  RegCount * sizeof (MEMORY_REGION_DESCRIPTION),
                                  (RegCount + Amount) * sizeof (MEMORY_REGION_DESCRIPTION),
                                  MemReg);
    if (TmpArray == NULL) {
      DEBUG ((
        EFI_D_ERROR,
        "%a: failed to allocate memory for memory region info, Status = %r\n",
        __func__,
        EFI_OUT_OF_RESOURCES
        ));
      if (MemReg != NULL) {
        FreePool (MemReg);
      }
      return EFI_OUT_OF_RESOURCES;
    }

    MemReg = TmpArray;

    for (Idx = 0; Idx < Amount; ++Idx, ++RegCount) {
      UINT64  Tmp;

      Tmp = SwapBytes32 (*Reg++);
      if (AddressCells > 1) {
        Tmp = (Tmp << 32) | SwapBytes32 (*Reg++);
      }
      MemReg[RegCount].Addr = Tmp;

      Tmp = SwapBytes32 (*Reg++);
      if (SizeCells > 1) {
        Tmp = (Tmp << 32) | SwapBytes32 (*Reg++);
      }
      MemReg[RegCount].Size = Tmp;
    }
  }

  *Regions = MemReg;
  *Count = RegCount;

  return EFI_SUCCESS;
}

EFI_STATUS
SratInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINTN                                                ChipIdx;
  UINTN                                                Idx;
  UINTN                                                Num;
  MEMORY_REGION_DESCRIPTION                           *Regions;
  UINTN                                                RegCount;
  UINTN                                                Size;
  EFI_ACPI_6_4_GICC_AFFINITY_STRUCTURE                *SratGiccAffinity;
  EFI_ACPI_6_4_GIC_ITS_AFFINITY_STRUCTURE             *SratGicItsAffinity;
  EFI_ACPI_6_4_MEMORY_AFFINITY_STRUCTURE              *SratMemAffinity;
  EFI_ACPI_6_4_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER  *SratTablePointer;
  EFI_STATUS                                           Status;

  Status = GetMemoryRegions (&Regions, &RegCount);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Size = sizeof (SratHeaderTemplate) +
         RegCount * sizeof (EFI_ACPI_6_4_MEMORY_AFFINITY_STRUCTURE) +
         PLATFORM_CHIP_COUNT * BS1000_CORE_COUNT * sizeof (EFI_ACPI_6_4_GICC_AFFINITY_STRUCTURE) +
         PLATFORM_CHIP_COUNT * BS1000_GIC_ITS_COUNT * sizeof (EFI_ACPI_6_4_GIC_ITS_AFFINITY_STRUCTURE);

  /* SRAT Table Header */
  SratTablePointer = (EFI_ACPI_6_4_SYSTEM_RESOURCE_AFFINITY_TABLE_HEADER *) AllocateZeroPool (Size);
  if (SratTablePointer == NULL) {
    DEBUG ((
      EFI_D_ERROR,
      "%a: failed to allocate memory, Status = %r\n",
      __func__,
      EFI_OUT_OF_RESOURCES
      ));
    if (Regions != NULL) {
      FreePool (Regions);
    }
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem ((VOID *) SratTablePointer, (VOID *) &SratHeaderTemplate, sizeof (SratHeaderTemplate));
  SratTablePointer->Header.Length = Size;

  /* Memory Affinity Structures */
  SratMemAffinity =
    (EFI_ACPI_6_4_MEMORY_AFFINITY_STRUCTURE *) ((UINT8 *) SratTablePointer + sizeof (SratHeaderTemplate));
  for (Idx = 0; Idx < RegCount; ++Idx) {
    ZeroMem ((VOID *) &SratMemAffinity[Idx], sizeof (SratMemAffinity[Idx]));
    SratMemAffinity[Idx].Type = EFI_ACPI_6_4_MEMORY_AFFINITY;
    SratMemAffinity[Idx].Length = sizeof (EFI_ACPI_6_4_MEMORY_AFFINITY_STRUCTURE);
    SratMemAffinity[Idx].ProximityDomain = (UINT32) PLATFORM_ADDR_CHIP (Regions[Idx].Addr);
    SratMemAffinity[Idx].AddressBaseLow =
      (UINT32) (Regions[Idx].Addr & 0xFFFFFFFF);
    SratMemAffinity[Idx].AddressBaseHigh =
      (UINT32) ((Regions[Idx].Addr & 0xFFFFFFFF00000000ULL) >> 32);
    SratMemAffinity[Idx].LengthLow =
      (UINT32) (Regions[Idx].Size & 0xFFFFFFFF);
    SratMemAffinity[Idx].LengthHigh =
      (UINT32) ((Regions[Idx].Size & 0xFFFFFFFF00000000ULL) >> 32);
    SratMemAffinity[Idx].Flags = EFI_ACPI_6_4_MEMORY_ENABLED;
  }

  /* Gic Core Affinity Structures */
  SratGiccAffinity =
    (EFI_ACPI_6_4_GICC_AFFINITY_STRUCTURE *) ((UINT8 *) SratMemAffinity +
      RegCount * sizeof (EFI_ACPI_6_4_MEMORY_AFFINITY_STRUCTURE));
  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_CORE_COUNT; ++Idx, ++Num) {
      ZeroMem ((VOID *) &SratGiccAffinity[Num], sizeof (SratGiccAffinity[Num]));
      SratGiccAffinity[Num].Type = EFI_ACPI_6_4_GICC_AFFINITY;
      SratGiccAffinity[Num].Length = sizeof (EFI_ACPI_6_4_GICC_AFFINITY_STRUCTURE);
      SratGiccAffinity[Num].ProximityDomain = ChipIdx;
      SratGiccAffinity[Num].AcpiProcessorUid = Num;
      SratGiccAffinity[Num].Flags = EFI_ACPI_6_4_GICC_ENABLED;
    }
  }

  /* Gic Its Affinity Structures */
  SratGicItsAffinity =
    (EFI_ACPI_6_4_GIC_ITS_AFFINITY_STRUCTURE *) ((UINT8 *) SratGiccAffinity +
      PLATFORM_CHIP_COUNT * BS1000_CORE_COUNT * sizeof (EFI_ACPI_6_4_GICC_AFFINITY_STRUCTURE));
  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_GIC_ITS_COUNT; ++Idx, ++Num) {
      ZeroMem ((VOID *) &SratGicItsAffinity[Num], sizeof (SratGicItsAffinity[Num]));
      SratGicItsAffinity[Num].Type = EFI_ACPI_6_4_GIC_ITS_AFFINITY;
      SratGicItsAffinity[Num].Length = sizeof (EFI_ACPI_6_4_GIC_ITS_AFFINITY_STRUCTURE);
      SratGicItsAffinity[Num].ProximityDomain = ChipIdx;
      SratGicItsAffinity[Num].ItsId = Num;
    }
  }

  if (Regions != NULL) {
    FreePool (Regions);
  }

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) SratTablePointer;
  return EFI_SUCCESS;
}

#else

EFI_STATUS
SratInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  return EFI_UNSUPPORTED;
}

#endif

VOID
SratDestroy (
  EFI_ACPI_DESCRIPTION_HEADER  *Table
  )
{
  ASSERT (Table != NULL);

  FreePool (Table);
}
