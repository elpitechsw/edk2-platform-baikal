/** @file
  Copyright (c) 2021 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef ACPI_PLATFORM_H_
#define ACPI_PLATFORM_H_

#define BAIKAL_ACPI_HEADER(Signature, Type, Revision, Id)  { \
  /* UINT32  Signature       */                              \
  Signature,                                                 \
  /* UINT32  Length          */                              \
  sizeof (Type),                                             \
  /* UINT8   Revision        */                              \
  Revision,                                                  \
  /* UINT8   Checksum        */                              \
  0,                                                         \
  /* UINT8   OemId[6]        */                              \
  { 'B', 'A', 'I', 'K', 'A', 'L' },                          \
  /* UINT64  OemTableId      */                              \
  ((UINT64)(Id) << 32) | 0x454C4B42,                         \
  /* UINT32  OemRevision     */                              \
  1,                                                         \
  /* UINT32  CreatorId       */                              \
  0x454C4B42,                                                \
  /* UINT32  CreatorRevision */                              \
  1                                                          \
}

#ifdef ELPITECH
#if defined(ELP_6)
#define BAIKAL_ACPI_PCIE_COUNT        11

#define BAIKAL_ACPI_PCIE0_P0_SEGMENT  0
#define BAIKAL_ACPI_PCIE1_P0_SEGMENT  2
#define BAIKAL_ACPI_PCIE2_P0_SEGMENT  4
#define BAIKAL_ACPI_PCIE2_P1_SEGMENT  5
#define BAIKAL_ACPI_PCIE3_P0_SEGMENT  6
#define BAIKAL_ACPI_PCIE3_P1_SEGMENT  7
#define BAIKAL_ACPI_PCIE3_P2_SEGMENT  8
#define BAIKAL_ACPI_PCIE3_P3_SEGMENT  9
#define BAIKAL_ACPI_PCIE4_P0_SEGMENT  10
#define BAIKAL_ACPI_PCIE4_P2_SEGMENT  12
#define BAIKAL_ACPI_PCIE4_P3_SEGMENT  13
#elif defined(ELP_8)
#define BAIKAL_ACPI_PCIE_COUNT        11

#define BAIKAL_ACPI_PCIE0_P0_SEGMENT  0
#define BAIKAL_ACPI_PCIE1_P0_SEGMENT  2
#define BAIKAL_ACPI_PCIE1_P1_SEGMENT  3
#define BAIKAL_ACPI_PCIE2_P0_SEGMENT  4
#define BAIKAL_ACPI_PCIE2_P1_SEGMENT  5
#define BAIKAL_ACPI_PCIE3_P1_SEGMENT  7
#define BAIKAL_ACPI_PCIE3_P2_SEGMENT  8
#define BAIKAL_ACPI_PCIE3_P3_SEGMENT  9
#define BAIKAL_ACPI_PCIE4_P0_SEGMENT  10
#define BAIKAL_ACPI_PCIE4_P2_SEGMENT  12
#define BAIKAL_ACPI_PCIE4_P3_SEGMENT  13
#endif
#else
#define BAIKAL_ACPI_PCIE_COUNT        14

#define BAIKAL_ACPI_PCIE0_P0_SEGMENT  0
#define BAIKAL_ACPI_PCIE0_P1_SEGMENT  1
#define BAIKAL_ACPI_PCIE1_P0_SEGMENT  2
#define BAIKAL_ACPI_PCIE1_P1_SEGMENT  3
#define BAIKAL_ACPI_PCIE2_P0_SEGMENT  4
#define BAIKAL_ACPI_PCIE2_P1_SEGMENT  5
#define BAIKAL_ACPI_PCIE3_P0_SEGMENT  6
#define BAIKAL_ACPI_PCIE3_P1_SEGMENT  7
#define BAIKAL_ACPI_PCIE3_P2_SEGMENT  8
#define BAIKAL_ACPI_PCIE3_P3_SEGMENT  9
#define BAIKAL_ACPI_PCIE4_P0_SEGMENT  10
#define BAIKAL_ACPI_PCIE4_P1_SEGMENT  11
#define BAIKAL_ACPI_PCIE4_P2_SEGMENT  12
#define BAIKAL_ACPI_PCIE4_P3_SEGMENT  13
#endif

#define BAIKAL_ACPI_PCIE0_P0_APB_BASE    0x38D40000
#define BAIKAL_ACPI_PCIE0_P1_APB_BASE    0x38D41000
#define BAIKAL_ACPI_PCIE1_P0_APB_BASE    0x3CD40000
#define BAIKAL_ACPI_PCIE1_P1_APB_BASE    0x3CD41000
#define BAIKAL_ACPI_PCIE2_P0_APB_BASE    0x44D40000
#define BAIKAL_ACPI_PCIE2_P1_APB_BASE    0x44D41000
#define BAIKAL_ACPI_PCIE3_P0_APB_BASE    0x48D40000
#define BAIKAL_ACPI_PCIE3_P1_APB_BASE    0x48D41000
#define BAIKAL_ACPI_PCIE3_P2_APB_BASE    0x48D42000
#define BAIKAL_ACPI_PCIE3_P3_APB_BASE    0x48D43000
#define BAIKAL_ACPI_PCIE4_P0_APB_BASE    0x4CD40000
#define BAIKAL_ACPI_PCIE4_P1_APB_BASE    0x4CD41000
#define BAIKAL_ACPI_PCIE4_P2_APB_BASE    0x4CD42000
#define BAIKAL_ACPI_PCIE4_P3_APB_BASE    0x4CD43000
#define BAIKAL_ACPI_PCIE_APB_SIZE        0x1000

#define BAIKAL_ACPI_PCIE0_P0_CFG_BASE    0x70000000000
#define BAIKAL_ACPI_PCIE0_P0_CFG_MAX     0x7000FFFFFFF
#define BAIKAL_ACPI_PCIE0_P0_IO_OFFSET   0x70100000000
#define BAIKAL_ACPI_PCIE0_P0_MEM_OFFSET  0x701C0000000

#define BAIKAL_ACPI_PCIE0_P1_CFG_BASE    0x74000000000
#define BAIKAL_ACPI_PCIE0_P1_CFG_MAX     0x7400FFFFFFF
#define BAIKAL_ACPI_PCIE0_P1_IO_OFFSET   0x74100000000
#define BAIKAL_ACPI_PCIE0_P1_MEM_OFFSET  0x741C0000000

#define BAIKAL_ACPI_PCIE1_P0_CFG_BASE    0x78000000000
#define BAIKAL_ACPI_PCIE1_P0_CFG_MAX     0x7800FFFFFFF
#define BAIKAL_ACPI_PCIE1_P0_IO_OFFSET   0x78100000000
#define BAIKAL_ACPI_PCIE1_P0_MEM_OFFSET  0x781C0000000

#define BAIKAL_ACPI_PCIE1_P1_CFG_BASE    0x7C000000000
#define BAIKAL_ACPI_PCIE1_P1_CFG_MAX     0x7C00FFFFFFF
#define BAIKAL_ACPI_PCIE1_P1_IO_OFFSET   0x7C100000000
#define BAIKAL_ACPI_PCIE1_P1_MEM_OFFSET  0x7C1C0000000

#define BAIKAL_ACPI_PCIE2_P0_CFG_BASE    0x50000000000
#define BAIKAL_ACPI_PCIE2_P0_CFG_MAX     0x5000FFFFFFF
#define BAIKAL_ACPI_PCIE2_P0_IO_OFFSET   0x50100000000
#define BAIKAL_ACPI_PCIE2_P0_MEM_OFFSET  0x501C0000000

#define BAIKAL_ACPI_PCIE2_P1_CFG_BASE    0x54000000000
#define BAIKAL_ACPI_PCIE2_P1_CFG_MAX     0x5400FFFFFFF
#define BAIKAL_ACPI_PCIE2_P1_IO_OFFSET   0x54100000000
#define BAIKAL_ACPI_PCIE2_P1_MEM_OFFSET  0x541C0000000

#define BAIKAL_ACPI_PCIE3_P0_CFG_BASE    0x40000000000
#define BAIKAL_ACPI_PCIE3_P0_CFG_MAX     0x4000FFFFFFF
#define BAIKAL_ACPI_PCIE3_P0_IO_OFFSET   0x40100000000
#define BAIKAL_ACPI_PCIE3_P0_MEM_OFFSET  0x401C0000000

#define BAIKAL_ACPI_PCIE3_P1_CFG_BASE    0x44000000000
#define BAIKAL_ACPI_PCIE3_P1_CFG_MAX     0x4400FFFFFFF
#define BAIKAL_ACPI_PCIE3_P1_IO_OFFSET   0x44100000000
#define BAIKAL_ACPI_PCIE3_P1_MEM_OFFSET  0x441C0000000

#define BAIKAL_ACPI_PCIE3_P2_CFG_BASE    0x48000000000
#define BAIKAL_ACPI_PCIE3_P2_CFG_MAX     0x4800FFFFFFF
#define BAIKAL_ACPI_PCIE3_P2_IO_OFFSET   0x48100000000
#define BAIKAL_ACPI_PCIE3_P2_MEM_OFFSET  0x481C0000000

#define BAIKAL_ACPI_PCIE3_P3_CFG_BASE    0x4C000000000
#define BAIKAL_ACPI_PCIE3_P3_CFG_MAX     0x4C00FFFFFFF
#define BAIKAL_ACPI_PCIE3_P3_IO_OFFSET   0x4C100000000
#define BAIKAL_ACPI_PCIE3_P3_MEM_OFFSET  0x4C1C0000000

#define BAIKAL_ACPI_PCIE4_P0_CFG_BASE    0x60000000000
#define BAIKAL_ACPI_PCIE4_P0_CFG_MAX     0x6000FFFFFFF
#define BAIKAL_ACPI_PCIE4_P0_IO_OFFSET   0x60100000000
#define BAIKAL_ACPI_PCIE4_P0_MEM_OFFSET  0x601C0000000

#define BAIKAL_ACPI_PCIE4_P1_CFG_BASE    0x64000000000
#define BAIKAL_ACPI_PCIE4_P1_CFG_MAX     0x6400FFFFFFF
#define BAIKAL_ACPI_PCIE4_P1_IO_OFFSET   0x64100000000
#define BAIKAL_ACPI_PCIE4_P1_MEM_OFFSET  0x641C0000000

#define BAIKAL_ACPI_PCIE4_P2_CFG_BASE    0x68000000000
#define BAIKAL_ACPI_PCIE4_P2_CFG_MAX     0x6800FFFFFFF
#define BAIKAL_ACPI_PCIE4_P2_IO_OFFSET   0x68100000000
#define BAIKAL_ACPI_PCIE4_P2_MEM_OFFSET  0x681C0000000

#define BAIKAL_ACPI_PCIE4_P3_CFG_BASE    0x6C000000000
#define BAIKAL_ACPI_PCIE4_P3_CFG_MAX     0x6C00FFFFFFF
#define BAIKAL_ACPI_PCIE4_P3_IO_OFFSET   0x6C100000000
#define BAIKAL_ACPI_PCIE4_P3_MEM_OFFSET  0x6C1C0000000

#define BAIKAL_ACPI_PCIE_CFG_OFFSET  0
#define BAIKAL_ACPI_PCIE_CFG_SIZE    0x10000000
#define BAIKAL_ACPI_PCIE_IO_BASE     0
#define BAIKAL_ACPI_PCIE_IO_MAX      0xFFFFF
#define BAIKAL_ACPI_PCIE_IO_SIZE     0x100000
#define BAIKAL_ACPI_PCIE_MEM_BASE    0x40000000
#define BAIKAL_ACPI_PCIE_MEM_MAX     0x7FFFFFFF
#define BAIKAL_ACPI_PCIE_MEM_SIZE    0x40000000

#define BAIKAL_ACPI_PCIE_LTSSM_STATE_MASK  0x3F
#define BAIKAL_ACPI_PCIE_LTSSM_STATE_L0    0x11

#define NATIVE_PCIE_OSC                                                    \
  Name (SUPP, Zero) /* PCI _OSC Support Field value */                     \
  Name (CTRL, Zero) /* PCI _OSC Control Field value */                     \
                                                                           \
  Method (_OSC, 4) {                                                       \
    /* Check for proper UUID */                                            \
    If (LEqual (Arg0, ToUUID ("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) {  \
      /* Create DWord-adressable fields from the Capabilities Buffer */    \
      CreateDWordField (Arg3, Zero, CDW1)                                  \
      CreateDWordField (Arg3, 4, CDW2)                                     \
      CreateDWordField (Arg3, 8, CDW3)                                     \
                                                                           \
      /* Save Capabilities DWord2 & 3 */                                   \
      Store (CDW2, SUPP)                                                   \
      Store (CDW3, CTRL)                                                   \
                                                                           \
      /* Only allow native hot plug control if OS supports: */             \
      /* * ASPM */                                                         \
      /* * Clock PM */                                                     \
      /* * MSI/MSI-X */                                                    \
      If (LNotEqual (And (SUPP, 0x16), 0x16)) {                            \
        And (CTRL, 0x3E, CTRL) /* Mask bit 0 (and undefined bits) */       \
      }                                                                    \
                                                                           \
      /* Always allow native PME, AER and PCIe Capability Structure */     \
      /* control */                                                        \
                                                                           \
      /* Never allow SHPC and LTR */                                       \
      And (CTRL, 0x1D, CTRL)                                               \
                                                                           \
      /* Unknown revision */                                               \
      If (LNotEqual (Arg1, One)) {                                         \
        Or (CDW1, 0x08, CDW1)                                              \
      }                                                                    \
                                                                           \
      /* Capabilities bits were masked */                                  \
      If (LNotEqual (CDW3, CTRL)) {                                        \
        Or (CDW1, 0x10, CDW1)                                              \
      }                                                                    \
                                                                           \
      /* Update DWORD3 in the buffer */                                    \
      Store (CTRL, CDW3)                                                   \
      Return (Arg3)                                                        \
    } Else {                                                               \
      /* Unrecognized UUID */                                              \
      Or (CDW1, 4, CDW1)                                                   \
      Return (Arg3)                                                        \
    }                                                                      \
  }

#endif // ACPI_PLATFORM_H_
