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

#define BAIKAL_ACPI_PCIE0_SEGMENT   0
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
# define BAIKAL_ACPI_PCIE1_SEGMENT  1
# define BAIKAL_ACPI_PCIE2_SEGMENT  2
# define BAIKAL_ACPI_PCIE_COUNT     3
#else
# define BAIKAL_ACPI_PCIE2_SEGMENT  1
# define BAIKAL_ACPI_PCIE_COUNT     2
#endif

#define SYNTH_SEG(RealSegment, Bus)  ((RealSegment + 1) * 10 + Bus)
#define SYNTH_BUS_PER_SEG            5

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

#define PROD_BUS_BUF(Index)                                     \
  WordBusNumber (ResourceProducer,                              \
    MinFixed, MaxFixed, PosDecode, 0x0, 0, 0, 0, 1,,, RB ## Index)

#define QRES_BUF_SET(Index, Offset, Length, Translation)        \
  CreateQwordField (RBUF, RB ## Index._MIN, MI ## Index)        \
  CreateQwordField (RBUF, RB ## Index._MAX, MA ## Index)        \
  CreateQwordField (RBUF, RB ## Index._LEN, LE ## Index)        \
  CreateQwordField (RBUF, RB ## Index._TRA, TR ## Index)        \
  Store (Length, LE ## Index)                                   \
  Store (Offset, MI ## Index)                                   \
  Store (Translation, TR ## Index)                              \
  Add (MI ## Index, LE ## Index - 1, MA ## Index)

#define WRES_BUF_SET(Index, Offset, Length, Translation)        \
  CreateWordField (RBUF, RB ## Index._MIN, MI ## Index)         \
  CreateWordField (RBUF, RB ## Index._MAX, MA ## Index)         \
  CreateWordField (RBUF, RB ## Index._LEN, LE ## Index)         \
  CreateWordField (RBUF, RB ## Index._TRA, TR ## Index)         \
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
