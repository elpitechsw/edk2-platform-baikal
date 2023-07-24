/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/ArmLib.h>
#include <Library/PcdLib.h>

#include <Platform/ConfigVars.h>

#include "AcpiPlatform.h"

#define BAIKAL_MADT_CPU_COUNT  8
#define RESERVED_MADT_TYPE     0xFF

#define BAIKAL_MADT_GICC_ENTRY(CPUInterfaceNumber, Mpidr)  { \
  /* UINT8   Type                          */                \
  EFI_ACPI_6_4_GIC,                                          \
  /* UINT8   Length                        */                \
  sizeof (EFI_ACPI_6_4_GIC_STRUCTURE),                       \
  /* UINT16  Reserved                      */                \
  EFI_ACPI_RESERVED_WORD,                                    \
  /* UINT32  CPUInterfaceNumber            */                \
  0,                                                         \
  /* UINT32  AcpiProcessorUid              */                \
  CPUInterfaceNumber,                                        \
  /* UINT32  Flags                         */                \
  EFI_ACPI_6_4_GIC_ENABLED,                                  \
  /* UINT32  ParkingProtocolVersion        */                \
  0,                                                         \
  /* UINT32  PerformanceInterruptGsiv      */                \
  23,                                                        \
  /* UINT64  ParkedAddress                 */                \
  0,                                                         \
  /* UINT64  PhysicalBaseAddress           */                \
  0,                                                         \
  /* UINT64  GICV                          */                \
  0,                                                         \
  /* UINT64  GICH                          */                \
  0,                                                         \
  /* UINT32  VGICMaintenanceInterrupt      */                \
  25,                                                        \
  /* UINT64  GICRBaseAddress               */                \
  FixedPcdGet64 (PcdGicRedistributorsBase) +                 \
    (0x20000 * (CPUInterfaceNumber)),                        \
  /* UINT64  MPIDR                         */                \
  Mpidr,                                                     \
  /* UINT8   ProcessorPowerEfficiencyClass */                \
  0,                                                         \
  /* UINT8   Reserved2                     */                \
  EFI_ACPI_RESERVED_BYTE,                                    \
  /* UINT16  SpeOverflowInterrupt          */                \
  0                                                          \
}

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_4_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER  Table;
  EFI_ACPI_6_4_GIC_STRUCTURE                           GicC[BAIKAL_MADT_CPU_COUNT];
  EFI_ACPI_6_4_GIC_DISTRIBUTOR_STRUCTURE               GicD;
  EFI_ACPI_6_4_GIC_ITS_STRUCTURE                       GicIts;
} BAIKAL_ACPI_MADT;

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
  {
    BAIKAL_MADT_GICC_ENTRY (0, GET_MPID (0, 0)),
    BAIKAL_MADT_GICC_ENTRY (1, GET_MPID (0, 1)),
    BAIKAL_MADT_GICC_ENTRY (2, GET_MPID (1, 0)),
    BAIKAL_MADT_GICC_ENTRY (3, GET_MPID (1, 1)),
    BAIKAL_MADT_GICC_ENTRY (4, GET_MPID (2, 0)),
    BAIKAL_MADT_GICC_ENTRY (5, GET_MPID (2, 1)),
    BAIKAL_MADT_GICC_ENTRY (6, GET_MPID (3, 0)),
    BAIKAL_MADT_GICC_ENTRY (7, GET_MPID (3, 1))
  },
  {
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
  },
  {
    /* UINT8   Type                */
    EFI_ACPI_6_4_GIC_ITS,
    /* UINT8   Length              */
    sizeof (EFI_ACPI_6_4_GIC_ITS_STRUCTURE),
    /* UINT16  Reserved            */
    EFI_ACPI_RESERVED_WORD,
    /* UINT32  GicItsId            */
    0,
    /* UINT64  PhysicalBaseAddress */
    0x2D020000,
    /* UINT32  Reserved2           */
    EFI_ACPI_RESERVED_DWORD
  }
};
#pragma pack()

EFI_STATUS
MadtInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Madt;
  return EFI_SUCCESS;
}
