/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>
#include <Library/BaseMemoryLib.h>
#include "AcpiPlatform.h"

#include <BS1000.h>

STATIC EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE  BaseAddrStructTemplate = {
  /* UINT64  BaseAddress           */
  0,
  /* UINT16  PciSegmentGroupNumber */
  0,
  /* UINT8   StartBusNumber        */
  0,
  /* UINT8   EndBusNumber          */
  255,
  /* UINT32  Reserved              */
  EFI_ACPI_RESERVED_DWORD
};

#pragma pack(1)
typedef struct {
  EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER                         Header;
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE  Table[PLATFORM_CHIP_COUNT * BAIKAL_ACPI_PCIE_COUNT];
} BAIKAL_ACPI_MCFG;
#pragma pack()

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
};

STATIC struct {
  UINT16  Segment;
  UINT64  BaseAddr;
}  PcieCfg[BAIKAL_ACPI_PCIE_COUNT] = {
  {BAIKAL_ACPI_PCIE0_P0_SEGMENT, BAIKAL_ACPI_PCIE0_P0_CFG_BASE},
  {BAIKAL_ACPI_PCIE0_P1_SEGMENT, BAIKAL_ACPI_PCIE0_P1_CFG_BASE},
  {BAIKAL_ACPI_PCIE1_P0_SEGMENT, BAIKAL_ACPI_PCIE1_P0_CFG_BASE},
  {BAIKAL_ACPI_PCIE1_P1_SEGMENT, BAIKAL_ACPI_PCIE1_P1_CFG_BASE},
  {BAIKAL_ACPI_PCIE2_P0_SEGMENT, BAIKAL_ACPI_PCIE2_P0_CFG_BASE},
  {BAIKAL_ACPI_PCIE2_P1_SEGMENT, BAIKAL_ACPI_PCIE2_P1_CFG_BASE},
  {BAIKAL_ACPI_PCIE3_P0_SEGMENT, BAIKAL_ACPI_PCIE3_P0_CFG_BASE},
  {BAIKAL_ACPI_PCIE3_P1_SEGMENT, BAIKAL_ACPI_PCIE3_P1_CFG_BASE},
  {BAIKAL_ACPI_PCIE3_P2_SEGMENT, BAIKAL_ACPI_PCIE3_P2_CFG_BASE},
  {BAIKAL_ACPI_PCIE3_P3_SEGMENT, BAIKAL_ACPI_PCIE3_P3_CFG_BASE},
  {BAIKAL_ACPI_PCIE4_P0_SEGMENT, BAIKAL_ACPI_PCIE4_P0_CFG_BASE},
  {BAIKAL_ACPI_PCIE4_P1_SEGMENT, BAIKAL_ACPI_PCIE4_P1_CFG_BASE},
  {BAIKAL_ACPI_PCIE4_P2_SEGMENT, BAIKAL_ACPI_PCIE4_P2_CFG_BASE},
  {BAIKAL_ACPI_PCIE4_P3_SEGMENT, BAIKAL_ACPI_PCIE4_P3_CFG_BASE}
};

EFI_STATUS
McfgInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINTN  ChipIdx;
  UINTN  Idx;
  UINTN  Num;

  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BAIKAL_ACPI_PCIE_COUNT; ++Idx, ++Num) {
      CopyMem (&Mcfg.Table[Num], &BaseAddrStructTemplate, sizeof (BaseAddrStructTemplate));
      Mcfg.Table[Num].BaseAddress = PLATFORM_ADDR_OUT_CHIP (ChipIdx, PcieCfg[Idx].BaseAddr);
      Mcfg.Table[Num].PciSegmentGroupNumber = ChipIdx * BAIKAL_ACPI_PCIE_COUNT +
                                              PcieCfg[Idx].Segment;
    }
  }

  Mcfg.Header.Header.OemRevision = 2;

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Mcfg;
  return EFI_SUCCESS;
}
