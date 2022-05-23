/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
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

#define BAIKAL_ACPI_PCIE_COUNT             14

#define BAIKAL_ACPI_PCIE0_P0_SEGMENT       0
#define BAIKAL_ACPI_PCIE0_P1_SEGMENT       1
#define BAIKAL_ACPI_PCIE1_P0_SEGMENT       2
#define BAIKAL_ACPI_PCIE1_P1_SEGMENT       3
#define BAIKAL_ACPI_PCIE2_P0_SEGMENT       4
#define BAIKAL_ACPI_PCIE2_P1_SEGMENT       5
#define BAIKAL_ACPI_PCIE3_P0_SEGMENT       6
#define BAIKAL_ACPI_PCIE3_P1_SEGMENT       7
#define BAIKAL_ACPI_PCIE3_P2_SEGMENT       8
#define BAIKAL_ACPI_PCIE3_P3_SEGMENT       9
#define BAIKAL_ACPI_PCIE4_P0_SEGMENT       10
#define BAIKAL_ACPI_PCIE4_P1_SEGMENT       11
#define BAIKAL_ACPI_PCIE4_P2_SEGMENT       12
#define BAIKAL_ACPI_PCIE4_P3_SEGMENT       13

#define BAIKAL_ACPI_PCIE_MMIO_SIZE         0x80000000

#define BAIKAL_ACPI_PCIE0_P0_MMIO_BASE     0x70000000000
#define BAIKAL_ACPI_PCIE0_P1_MMIO_BASE     0x74000000000
#define BAIKAL_ACPI_PCIE1_P0_MMIO_BASE     0x78000000000
#define BAIKAL_ACPI_PCIE1_P1_MMIO_BASE     0x7c000000000
#define BAIKAL_ACPI_PCIE2_P0_MMIO_BASE     0x50000000000
#define BAIKAL_ACPI_PCIE2_P1_MMIO_BASE     0x54000000000
#define BAIKAL_ACPI_PCIE3_P0_MMIO_BASE     0x40000000000
#define BAIKAL_ACPI_PCIE3_P1_MMIO_BASE     0x44000000000
#define BAIKAL_ACPI_PCIE3_P2_MMIO_BASE     0x48000000000
#define BAIKAL_ACPI_PCIE3_P3_MMIO_BASE     0x4c000000000
#define BAIKAL_ACPI_PCIE4_P0_MMIO_BASE     0x60000000000
#define BAIKAL_ACPI_PCIE4_P1_MMIO_BASE     0x64000000000
#define BAIKAL_ACPI_PCIE4_P2_MMIO_BASE     0x68000000000
#define BAIKAL_ACPI_PCIE4_P3_MMIO_BASE     0x6c000000000

#define BAIKAL_ACPI_PCIE_PORTIO_SIZE       0x100000

#define BAIKAL_ACPI_PCIE0_P0_PORTIO_BASE   0x73feff00000
#define BAIKAL_ACPI_PCIE0_P1_PORTIO_BASE   0x77feff00000
#define BAIKAL_ACPI_PCIE1_P0_PORTIO_BASE   0x7bfeff00000
#define BAIKAL_ACPI_PCIE1_P1_PORTIO_BASE   0x7ffeff00000
#define BAIKAL_ACPI_PCIE2_P0_PORTIO_BASE   0x53feff00000
#define BAIKAL_ACPI_PCIE2_P1_PORTIO_BASE   0x57feff00000
#define BAIKAL_ACPI_PCIE3_P0_PORTIO_BASE   0x43feff00000
#define BAIKAL_ACPI_PCIE3_P1_PORTIO_BASE   0x47feff00000
#define BAIKAL_ACPI_PCIE3_P2_PORTIO_BASE   0x4bfeff00000
#define BAIKAL_ACPI_PCIE3_P3_PORTIO_BASE   0x4ffeff00000
#define BAIKAL_ACPI_PCIE4_P0_PORTIO_BASE   0x63feff00000
#define BAIKAL_ACPI_PCIE4_P1_PORTIO_BASE   0x67feff00000
#define BAIKAL_ACPI_PCIE4_P2_PORTIO_BASE   0x6bfeff00000
#define BAIKAL_ACPI_PCIE4_P3_PORTIO_BASE   0x6ffeff00000

#define BAIKAL_ACPI_PCIE0_P0_PORTIO_MIN    0
#define BAIKAL_ACPI_PCIE0_P1_PORTIO_MIN    0x100000
#define BAIKAL_ACPI_PCIE1_P0_PORTIO_MIN    0x200000
#define BAIKAL_ACPI_PCIE1_P1_PORTIO_MIN    0x300000
#define BAIKAL_ACPI_PCIE2_P0_PORTIO_MIN    0x400000
#define BAIKAL_ACPI_PCIE2_P1_PORTIO_MIN    0x500000
#define BAIKAL_ACPI_PCIE3_P0_PORTIO_MIN    0x600000
#define BAIKAL_ACPI_PCIE3_P1_PORTIO_MIN    0x700000
#define BAIKAL_ACPI_PCIE3_P2_PORTIO_MIN    0x800000
#define BAIKAL_ACPI_PCIE3_P3_PORTIO_MIN    0x900000
#define BAIKAL_ACPI_PCIE4_P0_PORTIO_MIN    0xa00000
#define BAIKAL_ACPI_PCIE4_P1_PORTIO_MIN    0xb00000
#define BAIKAL_ACPI_PCIE4_P2_PORTIO_MIN    0xc00000
#define BAIKAL_ACPI_PCIE4_P3_PORTIO_MIN    0xd00000

#define BAIKAL_ACPI_PCIE_CFG_SIZE          0x10000000

#define BAIKAL_ACPI_PCIE0_P0_CFG_BASE      0x73ff0000000
#define BAIKAL_ACPI_PCIE0_P1_CFG_BASE      0x77ff0000000
#define BAIKAL_ACPI_PCIE1_P0_CFG_BASE      0x7bff0000000
#define BAIKAL_ACPI_PCIE1_P1_CFG_BASE      0x7fff0000000
#define BAIKAL_ACPI_PCIE2_P0_CFG_BASE      0x53ff0000000
#define BAIKAL_ACPI_PCIE2_P1_CFG_BASE      0x57ff0000000
#define BAIKAL_ACPI_PCIE3_P0_CFG_BASE      0x43ff0000000
#define BAIKAL_ACPI_PCIE3_P1_CFG_BASE      0x47ff0000000
#define BAIKAL_ACPI_PCIE3_P2_CFG_BASE      0x4bff0000000
#define BAIKAL_ACPI_PCIE3_P3_CFG_BASE      0x4fff0000000
#define BAIKAL_ACPI_PCIE4_P0_CFG_BASE      0x63ff0000000
#define BAIKAL_ACPI_PCIE4_P1_CFG_BASE      0x67ff0000000
#define BAIKAL_ACPI_PCIE4_P2_CFG_BASE      0x6bff0000000
#define BAIKAL_ACPI_PCIE4_P3_CFG_BASE      0x6fff0000000

#define BAIKAL_ACPI_PCIE_APB_SIZE          0x1000

#define BAIKAL_ACPI_PCIE0_P0_APB_BASE      0x38D40000
#define BAIKAL_ACPI_PCIE0_P1_APB_BASE      0x38D41000
#define BAIKAL_ACPI_PCIE1_P0_APB_BASE      0x3CD40000
#define BAIKAL_ACPI_PCIE1_P1_APB_BASE      0x3CD41000
#define BAIKAL_ACPI_PCIE2_P0_APB_BASE      0x44D40000
#define BAIKAL_ACPI_PCIE2_P1_APB_BASE      0x44D41000
#define BAIKAL_ACPI_PCIE3_P0_APB_BASE      0x48D40000
#define BAIKAL_ACPI_PCIE3_P1_APB_BASE      0x48D41000
#define BAIKAL_ACPI_PCIE3_P2_APB_BASE      0x48D42000
#define BAIKAL_ACPI_PCIE3_P3_APB_BASE      0x48D43000
#define BAIKAL_ACPI_PCIE4_P0_APB_BASE      0x4CD40000
#define BAIKAL_ACPI_PCIE4_P1_APB_BASE      0x4CD41000
#define BAIKAL_ACPI_PCIE4_P2_APB_BASE      0x4CD42000
#define BAIKAL_ACPI_PCIE4_P3_APB_BASE      0x4CD43000

#define BAIKAL_ACPI_PCIE_LTSSM_STATE_MASK  0x3F
#define BAIKAL_ACPI_PCIE_LTSSM_STATE_L0    0x11

#define CONS_MEM_BUF(Index)                                     \
  QWordMemory (ResourceConsumer,,                               \
    MinFixed, MaxFixed, NonCacheable, ReadWrite,                \
    0x0, 0x0, 0x0, 0x0, 0x1,,, RB ## Index)

#define PROD_MEM_BUF(Index)                                     \
  QWordMemory (ResourceProducer,,                               \
    MinFixed, MaxFixed, NonCacheable, ReadWrite,                \
    0x0, 0x0, 0x0, 0x0, 0x1,,, RB ## Index)

#define PROD_IO_BUF(Index)                                      \
  QWordIO (ResourceProducer,                                    \
    MinFixed, MaxFixed, PosDecode, EntireRange,                 \
    0x0, 0x0, 0x0, 0x0, 0x1,,, RB ## Index, TypeTranslation, )

#define RES_BUF_SET(Index, Offset, Length, Translation)         \
  CreateQwordField (RBUF, RB ## Index._MIN, MI ## Index)        \
  CreateQwordField (RBUF, RB ## Index._MAX, MA ## Index)        \
  CreateQwordField (RBUF, RB ## Index._LEN, LE ## Index)        \
  CreateQwordField (RBUF, RB ## Index._TRA, TR ## Index)        \
  Store (Length, LE ## Index)                                   \
  Store (Offset, MI ## Index)                                   \
  Store (Translation, TR ## Index)                              \
  Add (MI ## Index, LE ## Index - 1, MA ## Index)

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
