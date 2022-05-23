/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/IoRemappingTable.h>

#include "AcpiPlatform.h"

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_0_IO_REMAPPING_TABLE     Table;
  EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE  Its;
  UINT32                              ItsIdentifier[16];
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
    1,                                                      \
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
      EFI_ACPI_6_4_IO_REMAPPING_TABLE_SIGNATURE,
      BAIKAL_ACPI_IORT,
      3, /* Revision */
      0x54524F49
      ),
    /* UINT32  NumNodes   */
    1 + BAIKAL_ACPI_PCIE_COUNT, /* ITS + Root Complexes */
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
    16
  },
  /* UINT32  ItsIdentifier */
  {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
  },
  {
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE0_P0_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE0_P1_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE1_P0_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE1_P1_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE2_P0_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE2_P1_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE3_P0_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE3_P1_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE3_P2_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE3_P3_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE4_P0_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE4_P1_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE4_P2_SEGMENT),
    BAIKAL_IORT_ROOT_COMPLEX(BAIKAL_ACPI_PCIE4_P3_SEGMENT)
  }
};
#pragma pack()

EFI_STATUS
IortInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Iort;
  return EFI_SUCCESS;
}
