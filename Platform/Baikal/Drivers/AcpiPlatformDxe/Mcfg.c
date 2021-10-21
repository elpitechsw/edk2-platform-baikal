/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>

#include "AcpiPlatformDxe.h"

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
}  BAIKAL_ACPI_MCFG;

STATIC BAIKAL_ACPI_MCFG  Mcfg = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_3_PCI_EXPRESS_MEMORY_MAPPED_CONFIGURATION_SPACE_BASE_ADDRESS_DESCRIPTION_TABLE_SIGNATURE,
      BAIKAL_ACPI_MCFG,
      EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE_REVISION,
      0x4746434d
      ),
    /* UINT64  Reserved */
    EFI_ACPI_RESERVED_QWORD
  },
  {
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE_X4_0_SEGMENT, 0x40000000),
#ifdef BAIKAL_DBM
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE_X4_1_SEGMENT, 0x50000000),
#endif
    BAIKAL_MCFG_TABLE_ENTRY(BAIKAL_ACPI_PCIE_X8_SEGMENT, 0x60000000)
  }
};
#pragma pack()

EFI_STATUS
McfgInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *)&Mcfg;
  return EFI_SUCCESS;
}
