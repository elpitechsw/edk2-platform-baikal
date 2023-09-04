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
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20) || defined(ELP_1) || defined(ELP_4) || defined(ELP_7) || defined(ELP_8)
# define BAIKAL_ACPI_PCIE1_SEGMENT  1
# define BAIKAL_ACPI_PCIE2_SEGMENT  2
# define BAIKAL_ACPI_PCIE_COUNT     3
#elif defined(ELP_3)
# define BAIKAL_ACPI_PCIE1_SEGMENT  1
# define BAIKAL_ACPI_PCIE_COUNT     2
#else
# define BAIKAL_ACPI_PCIE2_SEGMENT  1
# define BAIKAL_ACPI_PCIE_COUNT     2
#endif

#ifdef ELPITECH
#define BAIKAL_ACPI_PCIE0_CFG_BASE   0x4000000000
#define BAIKAL_ACPI_PCIE0_CFG_MAX    0x400FFFFFFF
#define BAIKAL_ACPI_PCIE0_IO_OFFSET  0x44FF00000
#define BAIKAL_ACPI_PCIE0_MEM_BASE   0x40000000
#define BAIKAL_ACPI_PCIE0_MEM_OFFSET 0x3C0000000
#define BAIKAL_ACPI_PCIE0_MEM_MAX    0x7FFFFFFF
#define BAIKAL_ACPI_PCIE0_MEM_SIZE   0x40000000

#define BAIKAL_ACPI_PCIE1_CFG_BASE   0x5000000000
#define BAIKAL_ACPI_PCIE1_CFG_MAX    0x500FFFFFFF
#define BAIKAL_ACPI_PCIE1_IO_OFFSET  0x50000000
#define BAIKAL_ACPI_PCIE1_MEM_BASE   0x40000000
#define BAIKAL_ACPI_PCIE1_MEM_OFFSET 0x4C0000000
#define BAIKAL_ACPI_PCIE1_MEM_MAX    0x7FFFFFFF
#define BAIKAL_ACPI_PCIE1_MEM_SIZE   0x40000000

#define BAIKAL_ACPI_PCIE2_CFG_BASE   0x6000000000
#define BAIKAL_ACPI_PCIE2_CFG_MAX    0x600FFFFFFF
#define BAIKAL_ACPI_PCIE2_IO_OFFSET  0x60000000
#define BAIKAL_ACPI_PCIE2_MEM_BASE   0x80000000
#define BAIKAL_ACPI_PCIE2_MEM_OFFSET 0x580000000
#define BAIKAL_ACPI_PCIE2_MEM_MAX    0xFFFFFFFF
#define BAIKAL_ACPI_PCIE2_MEM_SIZE   0x80000000
#else
#define BAIKAL_ACPI_PCIE0_CFG_BASE   0x4000000000
#define BAIKAL_ACPI_PCIE0_CFG_MAX    0x400FFFFFFF
#define BAIKAL_ACPI_PCIE0_IO_OFFSET  0x4FF00000
#define BAIKAL_ACPI_PCIE0_MEM_BASE   0x40100000
#define BAIKAL_ACPI_PCIE0_MEM_OFFSET 0
#define BAIKAL_ACPI_PCIE0_MEM_MAX    0x4FEFFFFF
#define BAIKAL_ACPI_PCIE0_MEM_SIZE   0xFE00000

#define BAIKAL_ACPI_PCIE1_CFG_BASE   0x5000000000
#define BAIKAL_ACPI_PCIE1_CFG_MAX    0x500FFFFFFF
#define BAIKAL_ACPI_PCIE1_IO_OFFSET  0x5FF00000
#define BAIKAL_ACPI_PCIE1_MEM_BASE   0x50000000
#define BAIKAL_ACPI_PCIE1_MEM_OFFSET 0
#define BAIKAL_ACPI_PCIE1_MEM_MAX    0x5FEFFFFF
#define BAIKAL_ACPI_PCIE1_MEM_SIZE   0xFF00000

#define BAIKAL_ACPI_PCIE2_CFG_BASE   0x6000000000
#define BAIKAL_ACPI_PCIE2_CFG_MAX    0x600FFFFFFF
#define BAIKAL_ACPI_PCIE2_IO_OFFSET  0x7FF00000
#define BAIKAL_ACPI_PCIE2_MEM_BASE   0x60000000
#define BAIKAL_ACPI_PCIE2_MEM_OFFSET 0
#define BAIKAL_ACPI_PCIE2_MEM_MAX    0x7FEFFFFF
#define BAIKAL_ACPI_PCIE2_MEM_SIZE   0x1FF00000
#endif

#define BAIKAL_ACPI_PCIE_CFG_OFFSET  0
#define BAIKAL_ACPI_PCIE_CFG_SIZE    0x10000000
#define BAIKAL_ACPI_PCIE_IO_BASE     0
#define BAIKAL_ACPI_PCIE_IO_MAX      0xFFFFF
#define BAIKAL_ACPI_PCIE_IO_SIZE     0x100000
#define BAIKAL_ACPI_PCIE_MEM_OFFSET  0

#ifdef ELPITECH
#define SYNTH_BUS_PER_SEG            1
#else
#define SYNTH_SEG(RealSegment, Bus)  ((RealSegment + 1) * 10 + Bus)
#define SYNTH_BUS_PER_SEG            5
#endif

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

#define ACPI_BAIKAL_SMC_CMU_DATA              \
  OperationRegion (SMCR, FFixedHW, One, 0x28) \
  Field (SMCR, BufferAcc, NoLock, Preserve)   \
  {                                           \
    SMCF, 0x140                               \
  }                                           \
  Name (SMCB, Buffer (0x28) {})               \
  CreateQWordField (SMCB, Zero, ID0)          \
  CreateQWordField (SMCB, 0x8, ID1)           \
  CreateQWordField (SMCB, 0x10, ID2)          \
  CreateQWordField (SMCB, 0x18, ID3)          \
  CreateQWordField (SMCB, 0x20, ID4)

#define ACPI_BAIKAL_SMC_CMU_CALL(Id1, Id2, Id3, Id4) \
  ID0 = 0xC2000000                                   \
  ID1 = Id1                                          \
  ID2 = Id2                                          \
  ID3 = Id3                                          \
  ID4 = Id4                                          \
  SMCB = (SMCF = SMCB)

#define ACPI_BAIKAL_CMU_PLL_SET_RATE(Cmu, RefFreq, Rate) \
  ACPI_BAIKAL_SMC_CMU_CALL (Cmu, 0, Rate, RefFreq)

#define ACPI_BAIKAL_CMU_PLL_GET_RATE(Cmu, RefFreq) \
  ACPI_BAIKAL_SMC_CMU_CALL (Cmu, 1, 0, RefFreq)

#define ACPI_BAIKAL_CMU_PLL_ENABLE(Cmu) \
  ACPI_BAIKAL_SMC_CMU_CALL (Cmu, 2, 0, 0)

#define ACPI_BAIKAL_CMU_PLL_DISABLE(Cmu)     \
  If (CSTA)                                  \
  {                                          \
    ACPI_BAIKAL_SMC_CMU_CALL (Cmu, 13, 0, 0) \
  }                                          \
  ACPI_BAIKAL_SMC_CMU_CALL (Cmu, 3, 0, 0)

#define ACPI_BAIKAL_CMU_PLL_DISABLE2(Cmu)    \
  ACPI_BAIKAL_SMC_CMU_CALL (Cmu, 3, 0, 0)

#define ACPI_BAIKAL_CMU_PLL_IS_ENABLED(Cmu) \
  ACPI_BAIKAL_SMC_CMU_CALL (Cmu, 5, 0, 0)

#define ACPI_BAIKAL_CMU_CLKCH_SET_RATE(Cmu, Channel, Rate) \
  ACPI_BAIKAL_SMC_CMU_CALL (Channel, 6, Rate, Cmu)

#define ACPI_BAIKAL_CMU_CLKCH_GET_RATE(Cmu, Channel) \
  ACPI_BAIKAL_SMC_CMU_CALL (Channel, 7, 0, Cmu)

#define ACPI_BAIKAL_CMU_CLKCH_ENABLE(Cmu, Channel) \
  ACPI_BAIKAL_SMC_CMU_CALL (Channel, 8, 0, Cmu)

#define ACPI_BAIKAL_CMU_CLKCH_DISABLE(Cmu, Channel) \
  If (CSTA)                                         \
  {                                                 \
    ACPI_BAIKAL_SMC_CMU_CALL (Channel, 14, 0, Cmu)  \
  }                                                 \
  ACPI_BAIKAL_SMC_CMU_CALL (Channel, 9, 0, Cmu)

#define ACPI_BAIKAL_CMU_CLKCH_DISABLE2(Cmu, Channel) \
  ACPI_BAIKAL_SMC_CMU_CALL (Channel, 9, 0, Cmu)

#define ACPI_BAIKAL_CMU_CLKCH_IS_ENABLED(Cmu, Channel) \
  ACPI_BAIKAL_SMC_CMU_CALL (Channel, 11, 0, Cmu)

#define ACPI_BAIKAL_PWM_PS_METHODS \
  Name (PSVL, 3)                   \
  Method (_PSC)                    \
  {                                \
    Return (PSVL)                  \
  }                                \
  Method (_PS0)                    \
  {                                \
    PSVL = 0                       \
  }                                \
  Method (_PS3)                    \
  {                                \
    PSVL = 3                       \
  }

#endif // ACPI_PLATFORM_H_
