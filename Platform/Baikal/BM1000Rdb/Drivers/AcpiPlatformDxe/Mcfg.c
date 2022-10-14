/** @file
  Copyright (c) 2021 - 2022, Andrei Warkentin <andreiw@mm.st>
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>
#include <Library/PcdLib.h>

#include <BM1000.h>
#include <Platform/ConfigVars.h>

#include "AcpiPlatform.h"

#define BAIKAL_MCFG_TABLE_ENTRY(Segment, Addr, EndBusNumber)  { \
  /* UINT64  BaseAddress           */                           \
  Addr,                                                         \
  /* UINT16  PciSegmentGroupNumber */                           \
  Segment,                                                      \
  /* UINT8   StartBusNumber        */                           \
  0,                                                            \
  /* UINT8   EndBusNumber          */                           \
  EndBusNumber,                                                 \
  /* UINT32  Reserved              */                           \
  EFI_ACPI_RESERVED_DWORD                                       \
}

#pragma pack(1)
typedef struct {
  EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER                         Header;
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE  Table[BAIKAL_ACPI_PCIE_COUNT * SYNTH_BUS_PER_SEG];
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
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE0_SEGMENT, BAIKAL_ACPI_PCIE0_CFG_BASE, 255),
#ifdef BAIKAL_ACPI_PCIE1_SEGMENT
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE1_SEGMENT, BAIKAL_ACPI_PCIE1_CFG_BASE, 255),
#endif
#ifdef BAIKAL_ACPI_PCIE2_SEGMENT
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE2_SEGMENT, BAIKAL_ACPI_PCIE2_CFG_BASE, 255),
#endif

    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE0_SEGMENT, 2), BAIKAL_ACPI_PCIE0_CFG_BASE + (2 << 20), 0),
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE0_SEGMENT, 3), BAIKAL_ACPI_PCIE0_CFG_BASE + (3 << 20), 0),
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE0_SEGMENT, 4), BAIKAL_ACPI_PCIE0_CFG_BASE + (4 << 20), 0),
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE0_SEGMENT, 5), BAIKAL_ACPI_PCIE0_CFG_BASE + (5 << 20), 0),
#ifdef BAIKAL_ACPI_PCIE1_SEGMENT
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE1_SEGMENT, 2), BAIKAL_ACPI_PCIE1_CFG_BASE + (2 << 20), 0),
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE1_SEGMENT, 3), BAIKAL_ACPI_PCIE1_CFG_BASE + (3 << 20), 0),
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE1_SEGMENT, 4), BAIKAL_ACPI_PCIE1_CFG_BASE + (4 << 20), 0),
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE1_SEGMENT, 5), BAIKAL_ACPI_PCIE1_CFG_BASE + (5 << 20), 0),
#endif
#ifdef BAIKAL_ACPI_PCIE2_SEGMENT
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE2_SEGMENT, 2), BAIKAL_ACPI_PCIE2_CFG_BASE + (2 << 20), 0),
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE2_SEGMENT, 3), BAIKAL_ACPI_PCIE2_CFG_BASE + (3 << 20), 0),
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE2_SEGMENT, 4), BAIKAL_ACPI_PCIE2_CFG_BASE + (4 << 20), 0),
    BAIKAL_MCFG_TABLE_ENTRY(SYNTH_SEG(BAIKAL_ACPI_PCIE2_SEGMENT, 5), BAIKAL_ACPI_PCIE2_CFG_BASE + (5 << 20), 0),
#endif
  }
};
#pragma pack()

EFI_STATUS
McfgInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  switch (PcdGet32 (PcdAcpiPcieMode)) {
  case ACPI_PCIE_CUSTOM:
    *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Mcfg;
    return EFI_SUCCESS;

  case ACPI_PCIE_ECAM:
    //
    // Single-device ECAM, which allows any upstream OS without a Baikal-specific
    // driver to consume at least simple-enough (single-device) adapters.
    //
    if ((PcdGet32 (PcdPcieCfg0FilteringWorks) & (1 << BM1000_PCIE0_IDX)) == 0) {
      Mcfg.Table[BAIKAL_ACPI_PCIE0_SEGMENT].BaseAddress += 0x8000;
    }

    Mcfg.Table[BAIKAL_ACPI_PCIE0_SEGMENT].StartBusNumber = 0;
    Mcfg.Table[BAIKAL_ACPI_PCIE0_SEGMENT].EndBusNumber = 0;
#ifdef BAIKAL_ACPI_PCIE1_SEGMENT
    if ((PcdGet32 (PcdPcieCfg0FilteringWorks) & (1 << BM1000_PCIE1_IDX)) == 0) {
      Mcfg.Table[BAIKAL_ACPI_PCIE1_SEGMENT].BaseAddress += 0x8000;
    }

    Mcfg.Table[BAIKAL_ACPI_PCIE1_SEGMENT].StartBusNumber = 0;
    Mcfg.Table[BAIKAL_ACPI_PCIE1_SEGMENT].EndBusNumber = 0;
#endif
#ifdef BAIKAL_ACPI_PCIE2_SEGMENT
    if ((PcdGet32 (PcdPcieCfg0FilteringWorks) & (1 << BM1000_PCIE2_IDX)) == 0) {
      Mcfg.Table[BAIKAL_ACPI_PCIE2_SEGMENT].BaseAddress += 0x8000;
    }

    Mcfg.Table[BAIKAL_ACPI_PCIE2_SEGMENT].StartBusNumber = 0;
    Mcfg.Table[BAIKAL_ACPI_PCIE2_SEGMENT].EndBusNumber = 0;
#endif
    *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Mcfg;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}
