/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/IoRemappingTable.h>
#include <Library/PcdLib.h>

#include <Platform/ConfigVars.h>
#include <Platform/Pcie.h>

#include "AcpiPlatform.h"

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_0_IO_REMAPPING_TABLE     Table;
  EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE  Its;
  UINT32                              ItsIdentifier;
  struct {
    EFI_ACPI_6_0_IO_REMAPPING_RC_NODE   Node;
    EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE  Map;
  } Rc[BAIKAL_ACPI_PCIE_COUNT];
} BAIKAL_ACPI_IORT;

#define BAIKAL_IORT_ROOT_COMPLEX(Segment)  {                \
  {                                                         \
    {                                                       \
      /* UINT8   Type          */                           \
      EFI_ACPI_IORT_TYPE_ROOT_COMPLEX,                      \
      /* UINT16  Length        */                           \
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE) +          \
        sizeof (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE),        \
      /* UINT8   Revision      */                           \
      3,                                                    \
      /* UINT32  Identifier    */                           \
      Segment + 1,                                          \
      /* UINT32  NumIdMappings */                           \
      1,                                                    \
      /* UINT32  IdReference   */                           \
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE)            \
    },                                                      \
    /* UINT32  CacheCoherent     */                         \
    0                                ,                      \
    /* UINT8   AllocationHints   */                         \
    0,                                                      \
    /* UINT16  Reserved          */                         \
    EFI_ACPI_RESERVED_WORD,                                 \
    /* UINT8   MemoryAccessFlags */                         \
    0,                                                      \
    /* UINT32  AtsAttribute      */                         \
    0,                                                      \
    /* UINT32  PciSegmentNumber  */                         \
    Segment,                                                \
    /* UINT8   MemoryAddressSize */                         \
    64,                                                     \
    /* UINT8   Reserved1[3]      */                         \
    {                                                       \
      EFI_ACPI_RESERVED_BYTE,                               \
      EFI_ACPI_RESERVED_BYTE,                               \
      EFI_ACPI_RESERVED_BYTE                                \
    }                                                       \
  },                                                        \
  {                                                         \
    /* UINT32  InputBase       */                           \
    0,                                                      \
    /* UINT32  NumIds          */                           \
    0xFFFF,                                                 \
    /* UINT32  OutputBase      */                           \
    ((Segment + 1) << 16),                                  \
    /* UINT32  OutputReference */                           \
    OFFSET_OF (BAIKAL_ACPI_IORT, Its),                      \
    /* UINT32  Flags           */                           \
    0                                                       \
  }                                                         \
}

STATIC BAIKAL_ACPI_IORT  Iort = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_3_IO_REMAPPING_TABLE_SIGNATURE,
      BAIKAL_ACPI_IORT,
      3, /* Revision */
      0x54524F49
      ),
    /* UINT32  NumNodes   */
    BAIKAL_ACPI_PCIE_COUNT + 1, /* ITS + Root Complexes */
    /* UINT32  NodeOffset */
    OFFSET_OF (BAIKAL_ACPI_IORT, Its),
    /* UINT32  Reserved   */
    EFI_ACPI_RESERVED_DWORD
  },
  {
    {
      /* UINT8   Type          */
      EFI_ACPI_IORT_TYPE_ITS_GROUP,
      /* UINT16  Length        */
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE) +
        sizeof (((BAIKAL_ACPI_IORT *)0)->ItsIdentifier),
      /* UINT8   Revision      */
      1,
      /* UINT32  Identifier    */
      0,
      /* UINT32  NumIdMappings */
      0,
      /* UINT32  IdReference   */
      0
    },
    /* UINT32  NumItsIdentifiers */
    1
  },
  /* UINT32  ItsIdentifier */
  0,
  {
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE0_SEGMENT),
#ifdef BAIKAL_DBM
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE1_SEGMENT),
#endif
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE2_SEGMENT)
  }
};
#pragma pack()

EFI_STATUS
IortInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  if (PcdGet32 (PcdAcpiMsiMode) != ACPI_MSI_ITS) {
    return EFI_NOT_FOUND;
  }

  switch (PcdGet32 (PcdAcpiPcieMode)) {
  case ACPI_PCIE_CUSTOM:
    *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Iort;
    return EFI_SUCCESS;

  case ACPI_PCIE_ECAM:
    //
    // With single-device ECAM, the device is shown to the OS as 00:00.XX, but
    // the actual device is 01:00.XX (if CFG0 filtering works) or 01:01.XX
    // if (CFG0 filtering doesn't).
    //
    // Because the device is accessed via CFG0 cycles, it matters how exactly
    // the windows is mapped: we map it with address 0, i.e. encoding bus 0,
    // so from the device's perspective, the Requester ID will be 00:00.XX
    // (if CFG0 filtering works) or 00:01.XX (if it doesn't).
    //
    // Thus, the RIDs generated will be (where SS is the segment number):
    // - 0xSS0008 for single-function devices (where CFG0 filtering doesn't work)
    // - 0xSS0000 for multi-function devices (where CFG0 filtering works)
    //
    if ((PcdGet32 (PcdPcieCfg0FilteringWorks) & (1 << BM1000_PCIE0_IDX)) == 0) {
      Iort.Rc[BAIKAL_ACPI_PCIE0_SEGMENT].Map.OutputBase += 0x8;
    }

    Iort.Rc[BAIKAL_ACPI_PCIE0_SEGMENT].Map.NumIds = 0x7;
#ifdef BAIKAL_DBM
    if ((PcdGet32 (PcdPcieCfg0FilteringWorks) & (1 << BM1000_PCIE1_IDX)) == 0) {
      Iort.Rc[BAIKAL_ACPI_PCIE1_SEGMENT].Map.OutputBase += 0x8;
    }

    Iort.Rc[BAIKAL_ACPI_PCIE1_SEGMENT].Map.NumIds = 0x7;
#endif
    if ((PcdGet32 (PcdPcieCfg0FilteringWorks) & (1 << BM1000_PCIE2_IDX)) == 0) {
      Iort.Rc[BAIKAL_ACPI_PCIE2_SEGMENT].Map.OutputBase += 0x8;
    }

    Iort.Rc[BAIKAL_ACPI_PCIE2_SEGMENT].Map.NumIds = 0x7;
    *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Iort;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}
