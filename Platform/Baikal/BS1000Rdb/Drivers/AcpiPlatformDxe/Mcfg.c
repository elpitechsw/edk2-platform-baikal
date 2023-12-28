/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>
#include <Library/PcdLib.h>

#include "AcpiPlatform.h"

#define BAIKAL_MCFG_TABLE_ENTRY(Segment, Addr)  { \
  /* UINT64  BaseAddress           */             \
  Addr,                                           \
  /* UINT16  PciSegmentGroupNumber */             \
  Segment,                                        \
  /* UINT8   StartBusNumber        */             \
  1,                                              \
  /* UINT8   EndBusNumber          */             \
  255,                                            \
  /* UINT32  Reserved              */             \
  EFI_ACPI_RESERVED_DWORD                         \
}

#pragma pack(1)
typedef struct {
  EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER                         Header;
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE  Table[BAIKAL_ACPI_PCIE_COUNT];
} BAIKAL_ACPI_MCFG;

STATIC BAIKAL_ACPI_MCFG  Mcfg = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_PCI_EXPRESS_MEMORY_MAPPED_CONFIGURATION_SPACE_BASE_ADDRESS_DESCRIPTION_TABLE_SIGNATURE,
      BAIKAL_ACPI_MCFG,
      EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE_REVISION,
      0x4746434D
      ),
    /* UINT64  Reserved */
    EFI_ACPI_RESERVED_QWORD
  },
  {
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE0_P0_SEGMENT, BAIKAL_ACPI_PCIE0_P0_CFG_BASE),
#ifdef BAIKAL_ACPI_PCIE0_P1_SEGMENT
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE0_P1_SEGMENT, BAIKAL_ACPI_PCIE0_P1_CFG_BASE),
#endif
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE1_P0_SEGMENT, BAIKAL_ACPI_PCIE1_P0_CFG_BASE),
#ifdef BAIKAL_ACPI_PCIE1_P1_SEGMENT
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE1_P1_SEGMENT, BAIKAL_ACPI_PCIE1_P1_CFG_BASE),
#endif
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE2_P0_SEGMENT, BAIKAL_ACPI_PCIE2_P0_CFG_BASE),
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE2_P1_SEGMENT, BAIKAL_ACPI_PCIE2_P1_CFG_BASE),
#ifdef BAIKAL_ACPI_PCIE3_P0_SEGMENT
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE3_P0_SEGMENT, BAIKAL_ACPI_PCIE3_P0_CFG_BASE),
#endif
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE3_P1_SEGMENT, BAIKAL_ACPI_PCIE3_P1_CFG_BASE),
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE3_P2_SEGMENT, BAIKAL_ACPI_PCIE3_P2_CFG_BASE),
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE3_P3_SEGMENT, BAIKAL_ACPI_PCIE3_P3_CFG_BASE),
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE4_P0_SEGMENT, BAIKAL_ACPI_PCIE4_P0_CFG_BASE),
#ifdef BAIKAL_ACPI_PCIE4_P1_SEGMENT
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE4_P1_SEGMENT, BAIKAL_ACPI_PCIE4_P1_CFG_BASE),
#endif
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE4_P2_SEGMENT, BAIKAL_ACPI_PCIE4_P2_CFG_BASE),
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE4_P3_SEGMENT, BAIKAL_ACPI_PCIE4_P3_CFG_BASE)
  }
};
#pragma pack()

EFI_STATUS
McfgInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINT32 PcieCfg0Quirk = PcdGet32 (PcdPcieCfg0Quirk);
  INTN   Idx;

  for (Idx = 0; Idx < BAIKAL_ACPI_PCIE_COUNT; Idx++) {
    if (PcieCfg0Quirk & (1 << Mcfg.Table[Idx].PciSegmentGroupNumber)) {
      Mcfg.Table[Idx].BaseAddress += 0x8000;
    }
  }
  Mcfg.Header.Header.OemRevision = 2;
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Mcfg;
  return EFI_SUCCESS;
}
