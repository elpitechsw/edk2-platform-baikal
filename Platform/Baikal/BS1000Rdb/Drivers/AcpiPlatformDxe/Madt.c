/** @file
  Copyright (c) 2021 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/ArmLib.h>
#include <Library/PcdLib.h>
#include "AcpiPlatform.h"

#define BAIKAL_MADT_CPU_COUNT  48

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
  (Mpidr) << 8,                                              \
  /* UINT8   ProcessorPowerEfficiencyClass */                \
  0,                                                         \
  /* UINT8   Reserved2                     */                \
  EFI_ACPI_RESERVED_BYTE,                                    \
  /* UINT16  SpeOverflowInterrupt          */                \
  0                                                          \
}

#define BAIKAL_MADT_GICC_ENTRY_CLUSTER(ClusterId)                        \
  BAIKAL_MADT_GICC_ENTRY (4 * (ClusterId), GET_MPID (ClusterId, 0)),     \
  BAIKAL_MADT_GICC_ENTRY (4 * (ClusterId) + 1, GET_MPID (ClusterId, 1)), \
  BAIKAL_MADT_GICC_ENTRY (4 * (ClusterId) + 2, GET_MPID (ClusterId, 2)), \
  BAIKAL_MADT_GICC_ENTRY (4 * (ClusterId) + 3, GET_MPID (ClusterId, 3))

#define BAIKAL_MADT_GIC_ITS_ENTRY(Id, Addr)  { \
  /* UINT8   Type                */            \
  EFI_ACPI_6_4_GIC_ITS,                        \
  /* UINT8   Length              */            \
  sizeof (EFI_ACPI_6_4_GIC_ITS_STRUCTURE),     \
  /* UINT16  Reserved            */            \
  EFI_ACPI_RESERVED_WORD,                      \
  /* UINT32  GicItsId            */            \
  Id,                                          \
  /* UINT64  PhysicalBaseAddress */            \
  Addr,                                        \
  /* UINT32  Reserved2           */            \
  EFI_ACPI_RESERVED_DWORD                      \
}

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_4_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER  Table;
  EFI_ACPI_6_4_GIC_STRUCTURE                           GicC[BAIKAL_MADT_CPU_COUNT];
  EFI_ACPI_6_4_GIC_DISTRIBUTOR_STRUCTURE               GicD;
  EFI_ACPI_6_4_GIC_ITS_STRUCTURE                       GicIts[16];
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
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(0),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(1),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(2),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(3),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(4),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(5),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(6),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(7),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(8),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(9),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(10),
    BAIKAL_MADT_GICC_ENTRY_CLUSTER(11)
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
    BAIKAL_MADT_GIC_ITS_ENTRY(0, 0x01040000),
    BAIKAL_MADT_GIC_ITS_ENTRY(1, 0x01060000),
    BAIKAL_MADT_GIC_ITS_ENTRY(2, 0x01080000),
    BAIKAL_MADT_GIC_ITS_ENTRY(3, 0x010A0000),
    BAIKAL_MADT_GIC_ITS_ENTRY(4, 0x010C0000),
    BAIKAL_MADT_GIC_ITS_ENTRY(5, 0x010E0000),
    BAIKAL_MADT_GIC_ITS_ENTRY(6, 0x01100000),
    BAIKAL_MADT_GIC_ITS_ENTRY(7, 0x01120000),
    BAIKAL_MADT_GIC_ITS_ENTRY(8, 0x01140000),
    BAIKAL_MADT_GIC_ITS_ENTRY(9, 0x01160000),
    BAIKAL_MADT_GIC_ITS_ENTRY(10, 0x01180000),
    BAIKAL_MADT_GIC_ITS_ENTRY(11, 0x011A0000),
    BAIKAL_MADT_GIC_ITS_ENTRY(12, 0x011C0000),
    BAIKAL_MADT_GIC_ITS_ENTRY(13, 0x011E0000),
    BAIKAL_MADT_GIC_ITS_ENTRY(14, 0x01200000),
    BAIKAL_MADT_GIC_ITS_ENTRY(15, 0x01220000)
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
