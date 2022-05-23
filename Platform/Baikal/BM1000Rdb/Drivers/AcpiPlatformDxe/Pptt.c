/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/BaseMemoryLib.h>

#include "AcpiPlatform.h"

#define BAIKAL_PPTT_CLUSTER_NODE_COUNT  4
#define BAIKAL_PPTT_CORE_NODE_COUNT     8

/*
  - package L3 cache
  - cluster L2 cache
  - core L1 data cache
  - core L1 instruction cache
*/
#define BAIKAL_PPTT_CACHE_COUNT          4
#define BAIKAL_PPTT_PACKAGE_CACHE_COUNT  1
#define BAIKAL_PPTT_CLUSTER_CACHE_COUNT  1
#define BAIKAL_PPTT_CORE_CACHE_COUNT     2

#define BAIKAL_PPTT_PROC_NODE_FLAGS( \
  PhysicalPackage,                   \
  AcpiProcessorIdValid,              \
  ProcessorIsAThread,                \
  NodeIsALeaf,                       \
  IdenticalImplementation            \
  )                                  \
(                                    \
  PhysicalPackage             |      \
  (AcpiProcessorIdValid << 1) |      \
  (ProcessorIsAThread << 2)   |      \
  (NodeIsALeaf << 3)          |      \
  (IdenticalImplementation << 4)     \
)

#define BAIKAL_PPTT_CACHE_ATTRIBUTES( \
  AllocationType,                     \
  CacheType,                          \
  WritePolicy                         \
  )                                   \
(                                     \
  AllocationType |                    \
  (CacheType << 2) |                  \
  (WritePolicy << 4)                  \
)

#define BAIKAL_PPTT_CLUSTER_NODE(Id)  {                                         \
  {                                                                             \
    /* UINT8                                        Type                     */ \
    EFI_ACPI_6_4_PPTT_TYPE_PROCESSOR,                                           \
    /* UINT8                                        Length                   */ \
    sizeof (BAIKAL_PPTT_NODE),                                                  \
    /* UINT8                                        Reserved[2]              */ \
    { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE },                         \
    /* EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR_FLAGS  Flags                    */ \
    {},                                                                         \
    /* UINT32                                       Parent                   */ \
    OFFSET_OF (BAIKAL_ACPI_PPTT, Package),                                      \
    /* UINT32                                       AcpiProcessorId          */ \
    Id,                                                                         \
    /* UINT32                                       NumberOfPrivateResources */ \
    BAIKAL_PPTT_CLUSTER_CACHE_COUNT                                             \
  },                                                                            \
  /* UINT32  Resource */                                                        \
  OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[1])                                        \
}

#define BAIKAL_PPTT_CORE_NODE(Id, ClusterId)  {                                 \
  {                                                                             \
    /* UINT8                                        Type                     */ \
    EFI_ACPI_6_4_PPTT_TYPE_PROCESSOR,                                           \
    /* UINT8                                        Length                   */ \
    sizeof (BAIKAL_PPTT_NODE2),                                                 \
    /* UINT8                                        Reserved[2]              */ \
    { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE },                         \
    /* EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR_FLAGS  Flags                    */ \
    {},                                                                         \
    /* UINT32                                       Parent                   */ \
    OFFSET_OF (BAIKAL_ACPI_PPTT, Cluster[ClusterId]),                           \
    /* UINT32                                       AcpiProcessorId          */ \
    Id,                                                                         \
    /* UINT32                                       NumberOfPrivateResources */ \
    BAIKAL_PPTT_CORE_CACHE_COUNT                                                \
  },                                                                            \
  /* UINT32  Resource[2] */                                                     \
  {                                                                             \
    OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[2]),                                     \
    OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[3])                                      \
  }                                                                             \
}

#define BAIKAL_PPTT_CACHE_NODE(Size, Associativity)  {                 \
  /* UINT8                                         Type             */ \
  EFI_ACPI_6_4_PPTT_TYPE_CACHE,                                        \
  /* UINT8                                         Length           */ \
  sizeof (EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE),                          \
  /* UINT8                                         Reserved[2]      */ \
  { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE },                  \
  /* EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE_FLAGS       Flags            */ \
  {},                                                                  \
  /* UINT32                                        NextLevelOfCache */ \
  0,                                                                   \
  /* UINT32                                        Size             */ \
  Size,                                                                \
  /* UINT32                                        NumberOfSets     */ \
  Size / (Associativity * 64),                                         \
  /* UINT8                                         Associativity    */ \
  Associativity,                                                       \
  /* EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE_ATTRIBUTES  Attributes       */ \
  {},                                                                  \
  /* UINT16                                        LineSize         */ \
  64                                                                   \
}

#pragma pack(1)
typedef struct {
  EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR  Node;
  UINT32                                 Resource;
} BAIKAL_PPTT_NODE;

typedef struct {
  EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR  Node;
  UINT32                                 Resource[2];
} BAIKAL_PPTT_NODE2;

typedef struct {
  EFI_ACPI_6_4_PROCESSOR_PROPERTIES_TOPOLOGY_TABLE_HEADER  Table;
  BAIKAL_PPTT_NODE                                         Package;
  BAIKAL_PPTT_NODE                                         Cluster[BAIKAL_PPTT_CLUSTER_NODE_COUNT];
  BAIKAL_PPTT_NODE2                                        Core[BAIKAL_PPTT_CORE_NODE_COUNT];
  EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE                        Cache[BAIKAL_PPTT_CACHE_COUNT];
} BAIKAL_ACPI_PPTT;

STATIC BAIKAL_ACPI_PPTT  Pptt = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_PROCESSOR_PROPERTIES_TOPOLOGY_TABLE_STRUCTURE_SIGNATURE,
      BAIKAL_ACPI_PPTT,
      EFI_ACPI_6_4_PROCESSOR_PROPERTIES_TOPOLOGY_TABLE_REVISION,
      0x54545050
      ),
  },
  {
    {
      /* UINT8                                        Type                     */
      EFI_ACPI_6_4_PPTT_TYPE_PROCESSOR,
      /* UINT8                                        Length                   */
      sizeof (BAIKAL_PPTT_NODE),
      /* UINT8                                        Reserved[2]              */
      { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE },
      /* EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR_FLAGS  Flags                    */
      {},
      /* UINT32                                       Parent                   */
      0,
      /* UINT32                                       AcpiProcessorId          */
      12,
      /* UINT32                                       NumberOfPrivateResources */
      BAIKAL_PPTT_PACKAGE_CACHE_COUNT
    },
    /* UINT32  Resource */
    OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[0])
  },
  {
    BAIKAL_PPTT_CLUSTER_NODE (8),
    BAIKAL_PPTT_CLUSTER_NODE (9),
    BAIKAL_PPTT_CLUSTER_NODE (10),
    BAIKAL_PPTT_CLUSTER_NODE (11)
  },
  {
    BAIKAL_PPTT_CORE_NODE (0, 0),
    BAIKAL_PPTT_CORE_NODE (1, 0),
    BAIKAL_PPTT_CORE_NODE (2, 1),
    BAIKAL_PPTT_CORE_NODE (3, 1),
    BAIKAL_PPTT_CORE_NODE (4, 2),
    BAIKAL_PPTT_CORE_NODE (5, 2),
    BAIKAL_PPTT_CORE_NODE (6, 3),
    BAIKAL_PPTT_CORE_NODE (7, 3)
  },
  {
    BAIKAL_PPTT_CACHE_NODE (0x800000, 16),
    BAIKAL_PPTT_CACHE_NODE (0x100000, 16),
    BAIKAL_PPTT_CACHE_NODE (0x8000, 2),
    BAIKAL_PPTT_CACHE_NODE (0xC000, 3)
  }
};
#pragma pack()

EFI_STATUS
PpttInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINT8   CacheAttributes;
  UINT8   Idx;
  UINT32  ProcNodeFlags;

  ProcNodeFlags = BAIKAL_PPTT_PROC_NODE_FLAGS (
    EFI_ACPI_6_4_PPTT_PACKAGE_PHYSICAL,
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_VALID,
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,
    EFI_ACPI_6_4_PPTT_NODE_IS_NOT_LEAF,
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_IDENTICAL
    );
  CopyMem (&Pptt.Package.Node.Flags, &ProcNodeFlags, sizeof (ProcNodeFlags));

  ProcNodeFlags = BAIKAL_PPTT_PROC_NODE_FLAGS (
    EFI_ACPI_6_4_PPTT_PACKAGE_NOT_PHYSICAL,
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_VALID,
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,
    EFI_ACPI_6_4_PPTT_NODE_IS_NOT_LEAF,
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_IDENTICAL
    );
  for (Idx = 0; Idx < BAIKAL_PPTT_CLUSTER_NODE_COUNT; ++Idx) {
    CopyMem (&Pptt.Cluster[Idx].Node.Flags, &ProcNodeFlags, sizeof (ProcNodeFlags));
  }

  ProcNodeFlags = BAIKAL_PPTT_PROC_NODE_FLAGS (
    EFI_ACPI_6_4_PPTT_PACKAGE_NOT_PHYSICAL,
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_VALID,
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,
    EFI_ACPI_6_4_PPTT_NODE_IS_LEAF,
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_NOT_IDENTICAL
    );
  for (Idx = 0; Idx < BAIKAL_PPTT_CORE_NODE_COUNT; ++Idx) {
    CopyMem (&Pptt.Core[Idx].Node.Flags, &ProcNodeFlags, sizeof (ProcNodeFlags));
  }

  CacheAttributes = BAIKAL_PPTT_CACHE_ATTRIBUTES (
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ_WRITE,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_UNIFIED,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
    );
  CopyMem (&Pptt.Cache[0].Attributes, &CacheAttributes, sizeof (CacheAttributes));
  CopyMem (&Pptt.Cache[1].Attributes, &CacheAttributes, sizeof (CacheAttributes));

  CacheAttributes = BAIKAL_PPTT_CACHE_ATTRIBUTES (
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ_WRITE,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_DATA,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
    );
  CopyMem (&Pptt.Cache[2].Attributes, &CacheAttributes, sizeof (CacheAttributes));

  CacheAttributes = BAIKAL_PPTT_CACHE_ATTRIBUTES (
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_INSTRUCTION,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
    );
  CopyMem (&Pptt.Cache[3].Attributes, &CacheAttributes, sizeof (CacheAttributes));

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Pptt;
  return EFI_SUCCESS;
}
