/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include "AcpiPlatform.h"

#include <BS1000.h>

STATIC EFI_ACPI_6_4_GIC_STRUCTURE  GiccTemplate = {
  /* UINT8   Type                          */
  EFI_ACPI_6_4_GIC,
  /* UINT8   Length                        */
  sizeof (EFI_ACPI_6_4_GIC_STRUCTURE),
  /* UINT16  Reserved                      */
  EFI_ACPI_RESERVED_WORD,
  /* UINT32  CPUInterfaceNumber            */
  0,
  /* UINT32  AcpiProcessorUid              */
  0,
  /* UINT32  Flags                         */
  EFI_ACPI_6_4_GIC_ENABLED,
  /* UINT32  ParkingProtocolVersion        */
  0,
  /* UINT32  PerformanceInterruptGsiv      */
  23,
  /* UINT64  ParkedAddress                 */
  0,
  /* UINT64  PhysicalBaseAddress           */
  0,
  /* UINT64  GICV                          */
  0,
  /* UINT64  GICH                          */
  0,
  /* UINT32  VGICMaintenanceInterrupt      */
  25,
  /* UINT64  GICRBaseAddress               */
  0,
  /* UINT64  MPIDR                         */
  0,
  /* UINT8   ProcessorPowerEfficiencyClass */
  0,
  /* UINT8   Reserved2                     */
  EFI_ACPI_RESERVED_BYTE,
  /* UINT16  SpeOverflowInterrupt          */
  0
};

STATIC EFI_ACPI_6_4_GIC_DISTRIBUTOR_STRUCTURE  GicDTemplate = {
    /* UINT8   Type                */
    EFI_ACPI_6_4_GICD,
    /* UINT8   Length              */
    sizeof (EFI_ACPI_6_4_GIC_DISTRIBUTOR_STRUCTURE),
    /* UINT16  Reserved1           */
    EFI_ACPI_RESERVED_WORD,
    /* UINT32  GicId               */
    0,
    /* UINT64  PhysicalBaseAddress */
    FixedPcdGet64 (PcdGicDistributorBase),
    /* UINT32  SystemVectorBase    */
    EFI_ACPI_RESERVED_DWORD,
    /* UINT8   GicVersion          */
    EFI_ACPI_6_4_GIC_V3,
    /* UINT8   Reserved2[3]        */
    { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE }
};

STATIC EFI_ACPI_6_4_GIC_ITS_STRUCTURE  GicItsTemplate = {
  /* UINT8   Type                */
  EFI_ACPI_6_4_GIC_ITS,
  /* UINT8   Length              */
  sizeof (EFI_ACPI_6_4_GIC_ITS_STRUCTURE),
  /* UINT16  Reserved            */
  EFI_ACPI_RESERVED_WORD,
  /* UINT32  GicItsId            */
  0,
  /* UINT64  PhysicalBaseAddress */
  0,
  /* UINT32  Reserved2           */
  EFI_ACPI_RESERVED_DWORD
};

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_4_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER  Header;
  EFI_ACPI_6_4_GIC_STRUCTURE                           GicC[PLATFORM_CHIP_COUNT * BS1000_CORE_COUNT];
  EFI_ACPI_6_4_GIC_DISTRIBUTOR_STRUCTURE               GicD;
  EFI_ACPI_6_4_GIC_ITS_STRUCTURE                       GicIts[PLATFORM_CHIP_COUNT * BS1000_GIC_ITS_COUNT];
} BAIKAL_ACPI_MADT;
#pragma pack()

STATIC BAIKAL_ACPI_MADT  Madt = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE,
      BAIKAL_ACPI_MADT,
      EFI_ACPI_6_4_MULTIPLE_APIC_DESCRIPTION_TABLE_REVISION,
      0x5444414D
      ),
    /* UINT32  LocalApicAddress */
    0,
    /* UINT32  Flags            */
    0
  },
};

EFI_STATUS
MadtInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINTN  ChipIdx;
  UINTN  Idx;
  UINTN  Num;

  /* Each core Gic interface */
  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_CORE_COUNT; ++Idx, ++Num) {
      EFI_ACPI_6_4_GIC_STRUCTURE  *GicCPointer = &Madt.GicC[Num];

      CopyMem (GicCPointer, &GiccTemplate, sizeof (GiccTemplate));
      GicCPointer->AcpiProcessorUid = Num;
      GicCPointer->GICRBaseAddress = PLATFORM_ADDR_OUT_CHIP (ChipIdx,
        FixedPcdGet64 (PcdGicRedistributorsBase) + 0x20000 * Idx);
      GicCPointer->MPIDR = GET_MPID (Idx / BS1000_CORE_COUNT_PER_CLUSTER,
                                     Idx % BS1000_CORE_COUNT_PER_CLUSTER) << 8;
      GicCPointer->MPIDR += (((UINT64) ChipIdx) << 32);
    }
  }

  /* Gic Distributor */
  CopyMem (&Madt.GicD, &GicDTemplate, sizeof (GicDTemplate));

  /* Gic ITS */
  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_GIC_ITS_COUNT; ++Idx, ++Num) {
      EFI_ACPI_6_4_GIC_ITS_STRUCTURE  *GicItsPointer = &Madt.GicIts[Num];

      CopyMem (GicItsPointer, &GicItsTemplate, sizeof (GicItsTemplate));
      GicItsPointer->GicItsId = Num;
      GicItsPointer->PhysicalBaseAddress = PLATFORM_ADDR_OUT_CHIP (ChipIdx, 0x01040000 + 0x20000 * Idx);
    }
  }

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Madt;
  return EFI_SUCCESS;
}
