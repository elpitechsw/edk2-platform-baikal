/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/IoRemappingTable.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include "AcpiPlatform.h"

#include <BS1000.h>

#define BAIKAL_IORT_ITS_COUNT  BS1000_GIC_ITS_COUNT

#pragma pack(1)

typedef struct {
  EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE  Node;
  UINT32                              ItsIdentifier;
} BAIKAL_ACPI_IORT_ITS_NODE;

typedef struct {
  EFI_ACPI_6_0_IO_REMAPPING_RC_NODE   Node;
  EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE  Map;
} BAIKAL_ACPI_IORT_RC_NODE;

typedef struct {
  EFI_ACPI_6_0_IO_REMAPPING_TABLE  Table;
  BAIKAL_ACPI_IORT_ITS_NODE        Its[PLATFORM_CHIP_COUNT * BAIKAL_IORT_ITS_COUNT];
  BAIKAL_ACPI_IORT_RC_NODE         Rc[PLATFORM_CHIP_COUNT * BAIKAL_ACPI_PCIE_COUNT];
} BAIKAL_ACPI_IORT;

#pragma pack()

STATIC BAIKAL_ACPI_IORT_ITS_NODE  ItsNodeTemplate = {
  {
    {
      /* UINT8   Type          */
      EFI_ACPI_IORT_TYPE_ITS_GROUP,
      /* UINT16  Length        */
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_ITS_NODE) + 4,
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
  0
};

STATIC BAIKAL_ACPI_IORT_RC_NODE  RcNodeTemplate = {
  {
    {
      /* UINT8   Type          */
      EFI_ACPI_IORT_TYPE_ROOT_COMPLEX,
      /* UINT16  Length        */
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE) +
        sizeof (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE),
      /* UINT8   Revision      */
      4,
      /* UINT32  Identifier    */
      0,
      /* UINT32  NumIdMappings */
      1,
      /* UINT32  IdReference   */
      sizeof (EFI_ACPI_6_0_IO_REMAPPING_RC_NODE)
    },
    /* UINT32  CacheCoherent     */
    0,
    /* UINT8   AllocationHints   */
    0,
    /* UINT16  Reserved          */
    EFI_ACPI_RESERVED_WORD,
    /* UINT8   MemoryAccessFlags */
    0,
    /* UINT32  AtsAttribute      */
    0,
    /* UINT32  PciSegmentNumber  */
    0,
    /* UINT8   MemoryAddressSize */
    64,
    /* UINT16  PasidCapabilities */
    0,
    /* UINT8   Reserved1[1]      */
    { EFI_ACPI_RESERVED_BYTE },
    /* UINT32  Flags             */
    0
  },
  {
    /* UINT32  InputBase       */
    0,
    /* UINT32  NumIds          */
    0xFFFF,
    /* UINT32  OutputBase      */
    0,
    /* UINT32  OutputReference */
    0,
    /* UINT32  Flags           */
    0
  }
};

STATIC BAIKAL_ACPI_IORT  Iort = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_IO_REMAPPING_TABLE_SIGNATURE,
      BAIKAL_ACPI_IORT,
      EFI_ACPI_IO_REMAPPING_TABLE_REVISION_05, /* Revision */
      0x54524F49
      ),
    /* UINT32  NumNodes   */
    PLATFORM_CHIP_COUNT * (BAIKAL_IORT_ITS_COUNT + BAIKAL_ACPI_PCIE_COUNT),
    /* UINT32  NodeOffset */
    OFFSET_OF (BAIKAL_ACPI_IORT, Its[0]),
    /* UINT32  Reserved   */
    EFI_ACPI_RESERVED_DWORD
  },
};

STATIC struct {
  UINT32  Segment;
  UINT32  ItsId;
  UINT32  BaseId;
}  PcieToItsMap[BAIKAL_ACPI_PCIE_COUNT] = {
  {BAIKAL_ACPI_PCIE0_P0_SEGMENT,  0, 0},
#ifdef BAIKAL_ACPI_PCIE0_P1_SEGMENT
  {BAIKAL_ACPI_PCIE0_P1_SEGMENT,  0, 1},
#endif
  {BAIKAL_ACPI_PCIE1_P0_SEGMENT,  2, 0},
#ifdef BAIKAL_ACPI_PCIE1_P1_SEGMENT
  {BAIKAL_ACPI_PCIE1_P1_SEGMENT,  2, 1},
#endif
  {BAIKAL_ACPI_PCIE2_P0_SEGMENT,  4, 0},
  {BAIKAL_ACPI_PCIE2_P1_SEGMENT,  4, 1},
  {BAIKAL_ACPI_PCIE3_P0_SEGMENT,  6, 0},
  {BAIKAL_ACPI_PCIE3_P1_SEGMENT,  7, 1},
  {BAIKAL_ACPI_PCIE3_P2_SEGMENT,  8, 2},
  {BAIKAL_ACPI_PCIE3_P3_SEGMENT,  9, 3},
  {BAIKAL_ACPI_PCIE4_P0_SEGMENT, 11, 0},
#ifdef BAIKAL_ACPI_PCIE4_P1_SEGMENT
  {BAIKAL_ACPI_PCIE4_P1_SEGMENT, 12, 1},
#endif
  {BAIKAL_ACPI_PCIE4_P2_SEGMENT, 13, 2},
  {BAIKAL_ACPI_PCIE4_P3_SEGMENT, 14, 3}
};

EFI_STATUS
IortInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINTN  ChipIdx;
  UINTN  Idx;
  UINTN  Num;
  UINT32 PcieCfg0Quirk = PcdGet32 (PcdPcieCfg0Quirk);

  /* ITS nodes */
  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BAIKAL_IORT_ITS_COUNT; ++Idx, ++Num) {
      BAIKAL_ACPI_IORT_ITS_NODE  *ItsNodePointer = &Iort.Its[Num];

      CopyMem (ItsNodePointer, &ItsNodeTemplate, sizeof (ItsNodeTemplate));
      ItsNodePointer->Node.Node.Identifier = Num;
      ItsNodePointer->ItsIdentifier = Num;
    }
  }

  /* RC nodes */
  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BAIKAL_ACPI_PCIE_COUNT; ++Idx, ++Num) {
      BAIKAL_ACPI_IORT_RC_NODE  *RcNodePointer = &Iort.Rc[Num];
      UINT32                     Segment = BAIKAL_ACPI_CHIP_PCIE_SEGMENT(ChipIdx,
                                             PcieToItsMap[Idx].Segment);
      UINT32                     ItsId = ChipIdx * BAIKAL_IORT_ITS_COUNT +
                                         PcieToItsMap[Idx].ItsId;

      CopyMem (RcNodePointer, &RcNodeTemplate, sizeof (RcNodeTemplate));
      RcNodePointer->Node.Node.Identifier = PLATFORM_CHIP_COUNT * BAIKAL_IORT_ITS_COUNT + Segment;
      RcNodePointer->Node.PciSegmentNumber = Segment;
      RcNodePointer->Map.OutputBase = PcieToItsMap[Idx].BaseId << 16;
      if (PcieCfg0Quirk & (1 << Segment))
        RcNodePointer->Map.OutputBase += 0x8;
      RcNodePointer->Map.OutputReference = OFFSET_OF (BAIKAL_ACPI_IORT, Its[ItsId]);
    }
  }

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Iort;
  return EFI_SUCCESS;
}
