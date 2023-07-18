/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/BaseMemoryLib.h>
#include "AcpiPlatform.h"

#define BAIKAL_PPTT_CLUSTER_NODE_COUNT  12
#define BAIKAL_PPTT_CORE_NODE_COUNT     48

/*
  - package L4 cache
  - cluster L3 cache
  - core L2 cache
  - core L1 data cache
  - core L1 instruction cache
*/
#define BAIKAL_PPTT_CACHE_COUNT          157
#define BAIKAL_PPTT_PACKAGE_CACHE_COUNT  1
#define BAIKAL_PPTT_CLUSTER_CACHE_COUNT  1
#define BAIKAL_PPTT_CORE_CACHE_COUNT     3

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
  OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[Id - 47])                                  \
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
    BAIKAL_PPTT_CORE_CACHE_COUNT - 1                                            \
  },                                                                            \
  /* UINT32  Resource[2] */                                                     \
  {                                                                             \
    OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[Id + 61]),                               \
    OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[Id + 109])                               \
  }                                                                             \
}

#define BAIKAL_PPTT_CORE_NODE_CLUSTER(Id, ClusterId) \
  BAIKAL_PPTT_CORE_NODE(Id, ClusterId),              \
  BAIKAL_PPTT_CORE_NODE(Id + 1, ClusterId),          \
  BAIKAL_PPTT_CORE_NODE(Id + 2, ClusterId),          \
  BAIKAL_PPTT_CORE_NODE(Id + 3, ClusterId)

#define BAIKAL_PPTT_CACHE_NODE(Size, Associativity, Id)  {             \
  /* UINT8                                         Type             */ \
  EFI_ACPI_6_4_PPTT_TYPE_CACHE,                                        \
  /* UINT8                                         Length           */ \
  sizeof (EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE),                          \
  /* UINT8                                         Reserved[2]      */ \
  { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE },                  \
  /* EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE_FLAGS       Flags            */ \
  {                                                                    \
    EFI_ACPI_6_4_PPTT_CACHE_SIZE_VALID,                                \
    EFI_ACPI_6_4_PPTT_NUMBER_OF_SETS_VALID,                            \
    EFI_ACPI_6_4_PPTT_ASSOCIATIVITY_VALID,                             \
    EFI_ACPI_6_4_PPTT_ALLOCATION_TYPE_VALID,                           \
    EFI_ACPI_6_4_PPTT_CACHE_TYPE_VALID,                                \
    EFI_ACPI_6_4_PPTT_WRITE_POLICY_VALID,                              \
    EFI_ACPI_6_4_PPTT_LINE_SIZE_VALID,                                 \
    EFI_ACPI_6_4_PPTT_CACHE_ID_VALID                                   \
  },                                                                   \
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
  64,                                                                  \
  /* UINT32                                        CacheId          */ \
  Id                                                                   \
}

#define BAIKAL_PPTT_48_CACHE_NODES(Size, Associativity, StartId) \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId),         \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 1),     \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 2),     \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 3),     \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 4),     \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 5),     \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 6),     \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 7),     \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 8),     \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 9),     \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 10),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 11),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 12),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 13),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 14),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 15),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 16),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 17),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 18),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 19),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 20),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 21),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 22),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 23),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 24),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 25),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 26),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 27),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 28),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 29),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 30),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 31),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 32),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 33),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 34),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 35),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 36),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 37),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 38),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 39),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 40),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 41),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 42),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 43),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 44),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 45),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 46),    \
  BAIKAL_PPTT_CACHE_NODE (Size, Associativity, StartId + 47)

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
      60,
      /* UINT32                                       NumberOfPrivateResources */
      BAIKAL_PPTT_PACKAGE_CACHE_COUNT
    },
    /* UINT32  Resource */
    OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[0])
  },
  {
    BAIKAL_PPTT_CLUSTER_NODE (48),
    BAIKAL_PPTT_CLUSTER_NODE (49),
    BAIKAL_PPTT_CLUSTER_NODE (50),
    BAIKAL_PPTT_CLUSTER_NODE (51),
    BAIKAL_PPTT_CLUSTER_NODE (52),
    BAIKAL_PPTT_CLUSTER_NODE (53),
    BAIKAL_PPTT_CLUSTER_NODE (54),
    BAIKAL_PPTT_CLUSTER_NODE (55),
    BAIKAL_PPTT_CLUSTER_NODE (56),
    BAIKAL_PPTT_CLUSTER_NODE (57),
    BAIKAL_PPTT_CLUSTER_NODE (58),
    BAIKAL_PPTT_CLUSTER_NODE (59)
  },
  {
    BAIKAL_PPTT_CORE_NODE_CLUSTER (0, 0),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (4, 1),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (8, 2),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (12, 3),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (16, 4),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (20, 5),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (24, 6),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (28, 7),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (32, 8),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (36, 9),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (40, 10),
    BAIKAL_PPTT_CORE_NODE_CLUSTER (44, 11)
  },
  {
    BAIKAL_PPTT_CACHE_NODE (0x2000000, 16, 1),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 2),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 3),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 4),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 5),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 6),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 7),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 8),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 9),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 10),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 11),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 12),
    BAIKAL_PPTT_CACHE_NODE (0x200000, 16, 13),
    BAIKAL_PPTT_48_CACHE_NODES (0x80000, 8, 14),
    BAIKAL_PPTT_48_CACHE_NODES (0x10000, 16, 62),
    BAIKAL_PPTT_48_CACHE_NODES (0x10000, 4, 110)
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
  for (Idx = 0; Idx < 61; ++Idx) {
    CopyMem (&Pptt.Cache[Idx].Attributes, &CacheAttributes, sizeof (CacheAttributes));
  }

  CacheAttributes = BAIKAL_PPTT_CACHE_ATTRIBUTES (
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ_WRITE,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_DATA,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
    );
  for (Idx = 61; Idx < 109; ++Idx) {
    CopyMem (&Pptt.Cache[Idx].Attributes, &CacheAttributes, sizeof (CacheAttributes));
    Pptt.Cache[Idx].NextLevelOfCache = OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[Idx - 48]);
  }

  CacheAttributes = BAIKAL_PPTT_CACHE_ATTRIBUTES (
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_INSTRUCTION,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
    );
  for (Idx = 109; Idx < BAIKAL_PPTT_CACHE_COUNT; ++Idx) {
    CopyMem (&Pptt.Cache[Idx].Attributes, &CacheAttributes, sizeof (CacheAttributes));
    Pptt.Cache[Idx].NextLevelOfCache = OFFSET_OF (BAIKAL_ACPI_PPTT, Cache[Idx - 96]);
  }

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Pptt;
  return EFI_SUCCESS;
}
