/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/ArmLib.h>
#include <Library/PcdLib.h>
#include "AcpiPlatform.h"

#define BAIKAL_MADT_CPU_COUNT  48

#define BAIKAL_MADT_GICC_ENTRY(CPUInterfaceNumber, Mpidr)  { \
  /* UINT8   Type                          */                \
  EFI_ACPI_6_3_GIC,                                          \
  /* UINT8   Length                        */                \
  sizeof (EFI_ACPI_6_3_GIC_STRUCTURE),                       \
  /* UINT16  Reserved                      */                \
  EFI_ACPI_RESERVED_WORD,                                    \
  /* UINT32  CPUInterfaceNumber            */                \
  0,                                                         \
  /* UINT32  AcpiProcessorUid              */                \
  CPUInterfaceNumber,                                        \
  /* UINT32  Flags                         */                \
  EFI_ACPI_6_3_GIC_ENABLED,                                  \
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
  0,                                                         \
  /* UINT64  MPIDR                         */                \
  (Mpidr) << 8,                                              \
  /* UINT8   ProcessorPowerEfficiencyClass */                \
  0,                                                         \
  /* UINT8   Reserved2                     */                \
  EFI_ACPI_RESERVED_BYTE,                                    \
  /* UINT16  SpeOverflowInterrupt          */                \
  0                                                          \
}

#define BAIKAL_MADT_GICC_ENTRY_CLUSTER(ClusterId)                      \
  BAIKAL_MADT_GICC_ENTRY (4 * ClusterId, GET_MPID (ClusterId, 0)),     \
  BAIKAL_MADT_GICC_ENTRY (4 * ClusterId + 1, GET_MPID (ClusterId, 1)), \
  BAIKAL_MADT_GICC_ENTRY (4 * ClusterId + 2, GET_MPID (ClusterId, 2)), \
  BAIKAL_MADT_GICC_ENTRY (4 * ClusterId + 3, GET_MPID (ClusterId, 3))

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_3_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER  Table;
  EFI_ACPI_6_3_GIC_STRUCTURE                           GicC[BAIKAL_MADT_CPU_COUNT];
  EFI_ACPI_6_3_GIC_DISTRIBUTOR_STRUCTURE               GicD;
  EFI_ACPI_6_3_GICR_STRUCTURE                          GicR;
/*EFI_ACPI_6_3_GIC_ITS_STRUCTURE                       GicIts; */
} BAIKAL_ACPI_MADT;

STATIC BAIKAL_ACPI_MADT  Madt = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_3_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE,
      BAIKAL_ACPI_MADT,
      EFI_ACPI_6_3_MULTIPLE_APIC_DESCRIPTION_TABLE_REVISION,
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
    EFI_ACPI_6_3_GICD,
    /* UINT8   Length              */
    sizeof (EFI_ACPI_6_3_GIC_DISTRIBUTOR_STRUCTURE),
    /* UINT16  Reserved1           */
    EFI_ACPI_RESERVED_WORD,
    /* UINT32  GicId               */
    0,
    /* UINT64  PhysicalBaseAddress */
    FixedPcdGet64 (PcdGicDistributorBase),
    /* UINT32  SystemVectorBase    */
    EFI_ACPI_RESERVED_DWORD,
    /* UINT8   GicVersion          */
    EFI_ACPI_6_3_GIC_V3,
    /* UINT8   Reserved2[3]        */
    { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE }
  },
  {
    /* UINT8   Type                      */
    EFI_ACPI_6_3_GICR,
    /* UINT8   Length                    */
    sizeof (EFI_ACPI_6_3_GICR_STRUCTURE),
    /* UINT16  Reserved                  */
    EFI_ACPI_RESERVED_WORD,
    /* UINT64  DiscoveryRangeBaseAddress */
    FixedPcdGet64 (PcdGicRedistributorsBase),
    /* UINT32  DiscoveryRangeLength      */
    0x00600000
  }
  /* TODO: ITS */
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
