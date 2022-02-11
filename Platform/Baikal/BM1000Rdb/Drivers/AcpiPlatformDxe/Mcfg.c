/** @file
  Copyright (c) 2021, Andrei Warkentin <andreiw@mm.st>
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>
#include <Library/PcdLib.h>

#include <Platform/ConfigVars.h>
#include <Platform/Pcie.h>

#include "AcpiPlatform.h"

#define BAIKAL_MCFG_TABLE_ENTRY(Segment, Addr)  { \
  /* UINT64  BaseAddress           */             \
  Addr,                                           \
  /* UINT16  PciSegmentGroupNumber */             \
  Segment,                                        \
  /* UINT8   StartBusNumber        */             \
  0,                                              \
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
      EFI_ACPI_6_3_PCI_EXPRESS_MEMORY_MAPPED_CONFIGURATION_SPACE_BASE_ADDRESS_DESCRIPTION_TABLE_SIGNATURE,
      BAIKAL_ACPI_MCFG,
      EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE_REVISION,
      0x4746434D
      ),
    /* UINT64  Reserved */
    EFI_ACPI_RESERVED_QWORD
  },
  {
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE0_SEGMENT, BM1000_PCIE0_CFG_BASE),
#ifdef BAIKAL_DBM
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE1_SEGMENT, BM1000_PCIE1_CFG_BASE),
#endif
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE2_SEGMENT, BM1000_PCIE2_CFG_BASE)
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
#ifdef BAIKAL_DBM
    if ((PcdGet32 (PcdPcieCfg0FilteringWorks) & (1 << BM1000_PCIE1_IDX)) == 0) {
      Mcfg.Table[BAIKAL_ACPI_PCIE1_SEGMENT].BaseAddress += 0x8000;
    }

    Mcfg.Table[BAIKAL_ACPI_PCIE1_SEGMENT].StartBusNumber = 0;
    Mcfg.Table[BAIKAL_ACPI_PCIE1_SEGMENT].EndBusNumber = 0;
#endif
    if ((PcdGet32 (PcdPcieCfg0FilteringWorks) & (1 << BM1000_PCIE2_IDX)) == 0) {
      Mcfg.Table[BAIKAL_ACPI_PCIE2_SEGMENT].BaseAddress += 0x8000;
    }

    Mcfg.Table[BAIKAL_ACPI_PCIE2_SEGMENT].StartBusNumber = 0;
    Mcfg.Table[BAIKAL_ACPI_PCIE2_SEGMENT].EndBusNumber = 0;
    *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Mcfg;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}
