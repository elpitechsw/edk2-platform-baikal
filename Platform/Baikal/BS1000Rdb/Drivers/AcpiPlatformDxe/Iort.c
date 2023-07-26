/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/IoRemappingTable.h>
#include <Library/PcdLib.h>

#include "AcpiPlatform.h"

#define BAIKAL_IORT_ITS_COUNT  16

#define BAIKAL_IORT_ITS(Id)  {                         \
  {                                                    \
    {                                                  \
      /* UINT8   Type          */                      \
      EFI_ACPI_IORT_TYPE_ITS_GROUP,                    \
      /* UINT16  Length        */                      \
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE) + 4, \
      /* UINT8   Revision      */                      \
      1,                                               \
      /* UINT32  Identifier    */                      \
      Id,                                              \
      /* UINT32  NumIdMappings */                      \
      0,                                               \
      /* UINT32  IdReference   */                      \
      0                                                \
    },                                                 \
    /* UINT32  NumItsIdentifiers */                    \
    1                                                  \
  },                                                   \
  /* UINT32  ItsIdentifier */                          \
  Id                                                   \
}

#define BAIKAL_IORT_ROOT_COMPLEX(Segment, Iort, Id)  { \
  {                                                    \
    {                                                  \
      /* UINT8   Type          */                      \
      EFI_ACPI_IORT_TYPE_ROOT_COMPLEX,                 \
      /* UINT16  Length        */                      \
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE) +     \
        sizeof (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE),   \
      /* UINT8   Revision      */                      \
      4,                                               \
      /* UINT32  Identifier    */                      \
      Segment + BAIKAL_IORT_ITS_COUNT,                 \
      /* UINT32  NumIdMappings */                      \
      1,                                               \
      /* UINT32  IdReference   */                      \
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE)       \
    },                                                 \
    /* UINT32  CacheCoherent     */                    \
    0,                                                 \
    /* UINT8   AllocationHints   */                    \
    0,                                                 \
    /* UINT16  Reserved          */                    \
    EFI_ACPI_RESERVED_WORD,                            \
    /* UINT8   MemoryAccessFlags */                    \
    0,                                                 \
    /* UINT32  AtsAttribute      */                    \
    0,                                                 \
    /* UINT32  PciSegmentNumber  */                    \
    Segment,                                           \
    /* UINT8   MemoryAddressSize */                    \
    64,                                                \
    /* UINT16  PasidCapabilities */                    \
    0,                                                 \
    /* UINT8   Reserved1[1]      */                    \
    { EFI_ACPI_RESERVED_BYTE },                        \
    /* UINT32  Flags             */                    \
    0                                                  \
  },                                                   \
  {                                                    \
    /* UINT32  InputBase       */                      \
    0,                                                 \
    /* UINT32  NumIds          */                      \
    0xFFFF,                                            \
    /* UINT32  OutputBase      */                      \
    ((Id) << 16),                                      \
    /* UINT32  OutputReference */                      \
    OFFSET_OF (BAIKAL_ACPI_IORT, Its[Iort]),           \
    /* UINT32  Flags           */                      \
    0                                                  \
  }                                                    \
}

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_0_IO_REMAPPING_TABLE  Table;
  struct {
    EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE  Node;
    UINT32                              ItsIdentifier;
  } Its[16];
  struct {
    EFI_ACPI_6_0_IO_REMAPPING_RC_NODE   Node;
    EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE  Map;
  } Rc[BAIKAL_ACPI_PCIE_COUNT];
} BAIKAL_ACPI_IORT;

STATIC BAIKAL_ACPI_IORT  Iort = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_IO_REMAPPING_TABLE_SIGNATURE,
      BAIKAL_ACPI_IORT,
      EFI_ACPI_IO_REMAPPING_TABLE_REVISION_05, /* Revision */
      0x54524F49
      ),
    /* UINT32  NumNodes   */
    BAIKAL_IORT_ITS_COUNT +
      BAIKAL_ACPI_PCIE_COUNT,
    /* UINT32  NodeOffset */
    OFFSET_OF (BAIKAL_ACPI_IORT, Its[0]),
    /* UINT32  Reserved   */
    EFI_ACPI_RESERVED_DWORD
  },
  {
    BAIKAL_IORT_ITS(0),
    BAIKAL_IORT_ITS(1),
    BAIKAL_IORT_ITS(2),
    BAIKAL_IORT_ITS(3),
    BAIKAL_IORT_ITS(4),
    BAIKAL_IORT_ITS(5),
    BAIKAL_IORT_ITS(6),
    BAIKAL_IORT_ITS(7),
    BAIKAL_IORT_ITS(8),
    BAIKAL_IORT_ITS(9),
    BAIKAL_IORT_ITS(10),
    BAIKAL_IORT_ITS(11),
    BAIKAL_IORT_ITS(12),
    BAIKAL_IORT_ITS(13),
    BAIKAL_IORT_ITS(14),
    BAIKAL_IORT_ITS(15)
  },
  {
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE0_P0_SEGMENT, 0, 0),
#ifdef BAIKAL_ACPI_PCIE0_P1_SEGMENT
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE0_P1_SEGMENT, 0, 1),
#endif
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE1_P0_SEGMENT, 2, 0),
#ifdef BAIKAL_ACPI_PCIE1_P1_SEGMENT
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE1_P1_SEGMENT, 2, 1),
#endif
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE2_P0_SEGMENT, 4, 0),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE2_P1_SEGMENT, 4, 1),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE3_P0_SEGMENT, 6, 0),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE3_P1_SEGMENT, 7, 1),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE3_P2_SEGMENT, 8, 2),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE3_P3_SEGMENT, 9, 3),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE4_P0_SEGMENT, 11, 0),
#ifdef BAIKAL_ACPI_PCIE4_P1_SEGMENT
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE4_P1_SEGMENT, 12, 1),
#endif
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE4_P2_SEGMENT, 13, 2),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE4_P3_SEGMENT, 14, 3)
  }
};
#pragma pack()

EFI_STATUS
IortInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINT32 PcieCfg0Quirk = PcdGet32 (PcdPcieCfg0Quirk);
  INTN   Idx;

  for (Idx = 0; Idx < BAIKAL_ACPI_PCIE_COUNT; Idx++) {
    if (PcieCfg0Quirk & (1 << Iort.Rc[Idx].Node.PciSegmentNumber)) {
      Iort.Rc[Idx].Map.OutputBase += 0x8;
    }
  }
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Iort;
  return EFI_SUCCESS;
}
