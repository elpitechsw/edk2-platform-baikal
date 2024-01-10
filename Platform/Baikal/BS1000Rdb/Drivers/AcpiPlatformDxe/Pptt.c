/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/BaseMemoryLib.h>
#include "AcpiPlatform.h"

#include <BS1000.h>

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

STATIC EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR  ProcessorTemplate = {
  /* UINT8                                        Type                     */
  EFI_ACPI_6_4_PPTT_TYPE_PROCESSOR,
  /* UINT8                                        Length                   */
  0,
  /* UINT8                                        Reserved[2]              */
  { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE },
  /* EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR_FLAGS  Flags                    */
  {0},
  /* UINT32                                       Parent                   */
  0,
  /* UINT32                                       AcpiProcessorId          */
  0,
  /* UINT32                                       NumberOfPrivateResources */
  0
};

STATIC EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE  CacheTemplate = {
  /* UINT8                                         Type             */
  EFI_ACPI_6_4_PPTT_TYPE_CACHE,
  /* UINT8                                         Length           */
  sizeof (EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE),
  /* UINT8                                         Reserved[2]      */
  { EFI_ACPI_RESERVED_BYTE, EFI_ACPI_RESERVED_BYTE },
  /* EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE_FLAGS       Flags            */
  {
    EFI_ACPI_6_4_PPTT_CACHE_SIZE_VALID,
    EFI_ACPI_6_4_PPTT_NUMBER_OF_SETS_VALID,
    EFI_ACPI_6_4_PPTT_ASSOCIATIVITY_VALID,
    EFI_ACPI_6_4_PPTT_ALLOCATION_TYPE_VALID,
    EFI_ACPI_6_4_PPTT_CACHE_TYPE_VALID,
    EFI_ACPI_6_4_PPTT_WRITE_POLICY_VALID,
    EFI_ACPI_6_4_PPTT_LINE_SIZE_VALID,
    EFI_ACPI_6_4_PPTT_CACHE_ID_VALID
  },
  /* UINT32                                        NextLevelOfCache */
  0,
  /* UINT32                                        Size             */
  0,
  /* UINT32                                        NumberOfSets     */
  0,
  /* UINT8                                         Associativity    */
  0,
  /* EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE_ATTRIBUTES  Attributes       */
  {0},
  /* UINT16                                        LineSize         */
  64,
  /* UINT32                                        CacheId          */
  0
};

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
  EFI_ACPI_6_4_PROCESSOR_PROPERTIES_TOPOLOGY_TABLE_HEADER  Header;
  BAIKAL_PPTT_NODE                                         Package[PLATFORM_CHIP_COUNT];
  BAIKAL_PPTT_NODE                                         Cluster[PLATFORM_CHIP_COUNT * BS1000_CLUSTER_COUNT];
  BAIKAL_PPTT_NODE2                                        Core[PLATFORM_CHIP_COUNT * BS1000_CORE_COUNT];
  /* Package L4 Cache */
  EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE                        CacheL4[PLATFORM_CHIP_COUNT];
  /* Cluster L3 Cache */
  EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE                        CacheL3[PLATFORM_CHIP_COUNT * BS1000_CLUSTER_COUNT];
  /* Core L2 Cache */
  EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE                        CacheL2[PLATFORM_CHIP_COUNT * BS1000_CORE_COUNT];
  /* Core L1 Data Cache */
  EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE                        CacheL1D[PLATFORM_CHIP_COUNT * BS1000_CORE_COUNT];
  /* Core L1 Instruction Cache */
  EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE                        CacheL1I[PLATFORM_CHIP_COUNT * BS1000_CORE_COUNT];
} BAIKAL_ACPI_PPTT;

#pragma pack()

STATIC BAIKAL_ACPI_PPTT  Pptt = {
  {
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_PROCESSOR_PROPERTIES_TOPOLOGY_TABLE_STRUCTURE_SIGNATURE,
      BAIKAL_ACPI_PPTT,
      EFI_ACPI_6_4_PROCESSOR_PROPERTIES_TOPOLOGY_TABLE_REVISION,
      0x54545050
      ),
  },
};

EFI_STATUS
PpttInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINT8   CacheAttributes;
  UINTN   ChipIdx;
  UINTN   ClusterNum;
  UINTN   Idx;
  UINTN   Idx1;
  UINTN   Num;
  UINTN   CacheNum = 1;
  UINT32  ProcNodeFlags;

  /* Sockets */
  ProcNodeFlags = BAIKAL_PPTT_PROC_NODE_FLAGS (
    EFI_ACPI_6_4_PPTT_PACKAGE_PHYSICAL,
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_INVALID,
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,
    EFI_ACPI_6_4_PPTT_NODE_IS_NOT_LEAF,
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_IDENTICAL
    );
  for (ChipIdx = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    BAIKAL_PPTT_NODE  *NodePointer = &Pptt.Package[ChipIdx];

    CopyMem (&NodePointer->Node, &ProcessorTemplate, sizeof (ProcessorTemplate));
    NodePointer->Node.Length = sizeof (BAIKAL_PPTT_NODE);
    CopyMem (&NodePointer->Node.Flags, &ProcNodeFlags, sizeof (ProcNodeFlags));
    NodePointer->Node.NumberOfPrivateResources = 1;
    NodePointer->Resource = OFFSET_OF (BAIKAL_ACPI_PPTT, CacheL4[ChipIdx]);
  }

  /* Clusters */
  ProcNodeFlags = BAIKAL_PPTT_PROC_NODE_FLAGS (
    EFI_ACPI_6_4_PPTT_PACKAGE_NOT_PHYSICAL,
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_VALID,
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,
    EFI_ACPI_6_4_PPTT_NODE_IS_NOT_LEAF,
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_IDENTICAL
    );
  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_CLUSTER_COUNT; ++Idx, ++Num) {
      BAIKAL_PPTT_NODE  *NodePointer = &Pptt.Cluster[Num];

      CopyMem (&NodePointer->Node, &ProcessorTemplate, sizeof (ProcessorTemplate));
      NodePointer->Node.Length = sizeof (BAIKAL_PPTT_NODE);
      CopyMem (&NodePointer->Node.Flags, &ProcNodeFlags, sizeof (ProcNodeFlags));
      NodePointer->Node.Parent = OFFSET_OF (BAIKAL_ACPI_PPTT, Package[ChipIdx]);
      NodePointer->Node.AcpiProcessorId = BAIKAL_ACPI_CLUSTER_ID(Num);
      NodePointer->Node.NumberOfPrivateResources = 1;
      NodePointer->Resource = OFFSET_OF (BAIKAL_ACPI_PPTT, CacheL3[Num]);
    }
  }

  /* Cores */
  ProcNodeFlags = BAIKAL_PPTT_PROC_NODE_FLAGS (
    EFI_ACPI_6_4_PPTT_PACKAGE_NOT_PHYSICAL,
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_VALID,
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,
    EFI_ACPI_6_4_PPTT_NODE_IS_LEAF,
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_NOT_IDENTICAL
    );
  for (ChipIdx = 0, Num = 0, ClusterNum = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_CLUSTER_COUNT; ++Idx, ++ClusterNum) {
      for (Idx1 = 0; Idx1 < BS1000_CORE_COUNT_PER_CLUSTER; ++Idx1, ++Num) {
        BAIKAL_PPTT_NODE2  *NodePointer = &Pptt.Core[Num];

        CopyMem (&NodePointer->Node, &ProcessorTemplate, sizeof (ProcessorTemplate));
        NodePointer->Node.Length = sizeof (BAIKAL_PPTT_NODE2);
        CopyMem (&NodePointer->Node.Flags, &ProcNodeFlags, sizeof (ProcNodeFlags));
        NodePointer->Node.Parent = OFFSET_OF (BAIKAL_ACPI_PPTT, Cluster[ClusterNum]);
        NodePointer->Node.AcpiProcessorId = Num;
        NodePointer->Node.NumberOfPrivateResources = 2;
        NodePointer->Resource[0] = OFFSET_OF (BAIKAL_ACPI_PPTT, CacheL1D[Num]);
        NodePointer->Resource[1] = OFFSET_OF (BAIKAL_ACPI_PPTT, CacheL1I[Num]);
      }
    }
  }

  /* L4 Caches */
  CacheAttributes = BAIKAL_PPTT_CACHE_ATTRIBUTES (
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ_WRITE,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_UNIFIED,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
    );
  for (ChipIdx = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx, ++CacheNum) {
    EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE  *NodePointer = &Pptt.CacheL4[ChipIdx];

    CopyMem (NodePointer, &CacheTemplate, sizeof (CacheTemplate));
    NodePointer->Size = 0x2000000;
    NodePointer->Associativity = 16;
    NodePointer->NumberOfSets = NodePointer->Size / (NodePointer->Associativity * 64);
    CopyMem (&NodePointer->Attributes, &CacheAttributes, sizeof (CacheAttributes));
    NodePointer->CacheId = CacheNum;
  }

  /* L3 Caches */
  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_CLUSTER_COUNT; ++Idx, ++Num, ++CacheNum) {
      EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE  *NodePointer = &Pptt.CacheL3[Num];

      CopyMem (NodePointer, &CacheTemplate, sizeof (CacheTemplate));
      NodePointer->Size = 0x200000;
      NodePointer->Associativity = 16;
      NodePointer->NumberOfSets = NodePointer->Size / (NodePointer->Associativity * 64);
      CopyMem (&NodePointer->Attributes, &CacheAttributes, sizeof (CacheAttributes));
      NodePointer->CacheId = CacheNum;
    }
  }

  /* L2 Caches */
  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_CORE_COUNT; ++Idx, ++Num, ++CacheNum) {
      EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE  *NodePointer = &Pptt.CacheL2[Num];

      CopyMem (NodePointer, &CacheTemplate, sizeof (CacheTemplate));
      NodePointer->Size = 0x80000;
      NodePointer->Associativity = 8;
      NodePointer->NumberOfSets = NodePointer->Size / (NodePointer->Associativity * 64);
      CopyMem (&NodePointer->Attributes, &CacheAttributes, sizeof (CacheAttributes));
      NodePointer->CacheId = CacheNum;
    }
  }

  /* L1D Caches */
  CacheAttributes = BAIKAL_PPTT_CACHE_ATTRIBUTES (
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ_WRITE,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_DATA,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
    );
  for (ChipIdx = 0, Num = 0, ClusterNum = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_CLUSTER_COUNT; ++Idx, ++ClusterNum) {
      for (Idx1 = 0; Idx1 < BS1000_CORE_COUNT_PER_CLUSTER; ++Idx1, ++Num, ++CacheNum) {
        EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE  *NodePointer = &Pptt.CacheL1D[Num];

        CopyMem (NodePointer, &CacheTemplate, sizeof (CacheTemplate));
        NodePointer->NextLevelOfCache = OFFSET_OF (BAIKAL_ACPI_PPTT, CacheL2[ClusterNum]);
        NodePointer->Size = 0x10000;
        NodePointer->Associativity = 16;
        NodePointer->NumberOfSets = NodePointer->Size / (NodePointer->Associativity * 64);
        CopyMem (&NodePointer->Attributes, &CacheAttributes, sizeof (CacheAttributes));
        NodePointer->CacheId = CacheNum;
      }
    }
  }

  /* L1I Caches */
  CacheAttributes = BAIKAL_PPTT_CACHE_ATTRIBUTES (
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_INSTRUCTION,
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK
    );
  for (ChipIdx = 0, Num = 0, ClusterNum = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_CLUSTER_COUNT; ++Idx, ++ClusterNum) {
      for (Idx1 = 0; Idx1 < BS1000_CORE_COUNT_PER_CLUSTER; ++Idx1, ++Num, ++CacheNum) {
        EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE  *NodePointer = &Pptt.CacheL1I[Num];

        CopyMem (NodePointer, &CacheTemplate, sizeof (CacheTemplate));
        NodePointer->NextLevelOfCache = OFFSET_OF (BAIKAL_ACPI_PPTT, CacheL2[ClusterNum]);
        NodePointer->Size = 0x10000;
        NodePointer->Associativity = 4;
        NodePointer->NumberOfSets = NodePointer->Size / (NodePointer->Associativity * 64);
        CopyMem (&NodePointer->Attributes, &CacheAttributes, sizeof (CacheAttributes));
        NodePointer->CacheId = CacheNum;
      }
    }
  }

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Pptt;
  return EFI_SUCCESS;
}
