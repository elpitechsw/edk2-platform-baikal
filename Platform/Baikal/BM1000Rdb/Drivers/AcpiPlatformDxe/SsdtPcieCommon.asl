/** @file
  Copyright (c) 2021, Andrei Warkentin <andreiw@mm.st><BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <BM1000.h>
#include <Platform/Pcie.h>

#include "AcpiPlatform.h"

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

#ifdef PCIE_SINGLE_DEVICE_BUS_RANGE
# define BUS_RES WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode, 0x0, 0, 0, 0, 1)
#else
# define BUS_RES WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode, 0x0, 0, 0xFF, 0, 0x100)
#endif // PCIE_FULL_BUS_RANGE

  External (\_SB.STA0, FieldUnitObj)
  External (\_SB.STA1, FieldUnitObj)
  External (\_SB.STA2, FieldUnitObj)

  Scope (_SB_) {
#ifdef PCIE_WITH_ECAM_RANGE
  Device (ECAM) {
    Name (_HID, EISAID ("PNP0C02"))
    Method (_CRS, 0, Serialized) {
      Name (RBUF, ResourceTemplate () {
        CONS_MEM_BUF(01)
#ifdef BAIKAL_DBM
        CONS_MEM_BUF(02)
#endif
        CONS_MEM_BUF(03)
      })

      RES_BUF_SET(01, BM1000_PCIE0_CFG_BASE, 0xFF00000, 0)
#ifdef BAIKAL_DBM
      RES_BUF_SET(02, BM1000_PCIE1_CFG_BASE, 0xFF00000, 0)
#endif
      RES_BUF_SET(03, BM1000_PCIE2_CFG_BASE, 0xFF00000, 0)

      Return (RBUF)
    }
  }
#endif // PCIE_WITH_ECAM_RANGE

  // PCIe0 (x4 #0)
  Device (PCI0) {
    Name (_HID, EISAID ("PNP0A08"))
    Name (_CID, EISAID ("PNP0A03"))
    Name (_UID, BAIKAL_ACPI_PCIE0_SEGMENT)
    Name (_CCA, One)
    Name (_SEG, BAIKAL_ACPI_PCIE0_SEGMENT)
    Name (_BBN, 0)

    OperationRegion (CFGS, SystemMemory, BM1000_PCIE0_CFG_BASE, 0x10)
    Field (CFGS, ByteAcc, NoLock, Preserve) {
      Offset (0xE),
      TYPE, 8
    }

    Method (_STA, 0, Serialized) {
      if ((\_SB.STA0 & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                  BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
        Return (0x0)
      }

#ifdef PCIE_SINGLE_DEVICE_BUS_RANGE
      //
      // If we're only exposing one device, then don't bother
      // exposing bridges as we'll never be able to access the EPs
      // hanging off of them.
      //
      if ((TYPE & 0x7F) != 0) {
        Return (0x0)
      }
#endif // PCIE_SINGLE_DEVICE_BUS_RANGE
      Return (0xF)
    }

    Method (_CRS, 0, Serialized) {
      Name (RBUF, ResourceTemplate () {
        BUS_RES
        PROD_MEM_BUF(01)
        PROD_IO_BUF(02)
      })

      RES_BUF_SET(01, BM1000_PCIE0_MMIO32_BASE,  BM1000_PCIE0_MMIO32_SIZE, 0)
      RES_BUF_SET(02, BM1000_PCIE0_PORTIO_MIN,   BM1000_PCIE0_PORTIO_SIZE,
                      BM1000_PCIE0_PORTIO_BASE - BM1000_PCIE0_PORTIO_MIN)

      Return (RBUF)
    }

#ifdef PCIE_NATIVE
    NATIVE_PCIE_OSC
#endif // PCIE_NATIVE
  }

#ifdef BAIKAL_DBM
  // PCIe1 (x4 #1)
  Device (PCI1) {
    Name (_HID, EISAID ("PNP0A08"))
    Name (_CID, EISAID ("PNP0A03"))
    Name (_UID, BAIKAL_ACPI_PCIE1_SEGMENT)
    Name (_CCA, One)
    Name (_SEG, BAIKAL_ACPI_PCIE1_SEGMENT)
    Name (_BBN, 0)

    OperationRegion (CFGS, SystemMemory, BM1000_PCIE1_CFG_BASE, 0x10)
    Field (CFGS, ByteAcc, NoLock, Preserve) {
      Offset (0xE),
      TYPE, 8
    }

    Method (_STA, 0, Serialized) {
      if ((\_SB.STA1 & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                  BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
        Return (0x0)
      }

#ifdef PCIE_SINGLE_DEVICE_BUS_RANGE
      //
      // If we're only exposing one device, then don't bother
      // exposing bridges as we'll never be able to access the EPs
      // hanging off of them.
      //
      if ((TYPE & 0x7F) != 0) {
        Return (0x0)
      }
#endif // PCIE_SINGLE_DEVICE_BUS_RANGE
      Return (0xF)
    }

    Method (_CRS, 0, Serialized) {
      Name (RBUF, ResourceTemplate () {
        BUS_RES
        PROD_MEM_BUF(01)
        PROD_IO_BUF(02)
      })

      RES_BUF_SET(01, BM1000_PCIE1_MMIO32_BASE,  BM1000_PCIE1_MMIO32_SIZE, 0)
      RES_BUF_SET(02, BM1000_PCIE1_PORTIO_MIN,   BM1000_PCIE1_PORTIO_SIZE,
                      BM1000_PCIE1_PORTIO_BASE - BM1000_PCIE1_PORTIO_MIN)
      Return (RBUF)
    }

#ifdef PCIE_NATIVE
    NATIVE_PCIE_OSC
#endif // PCIE_NATIVE
  }
#endif

  // PCIe2 (x8)
  Device (PCI2) {
    Name (_HID, EISAID ("PNP0A08"))
    Name (_CID, EISAID ("PNP0A03"))
    Name (_UID, BAIKAL_ACPI_PCIE2_SEGMENT)
    Name (_CCA, One)
    Name (_SEG, BAIKAL_ACPI_PCIE2_SEGMENT)
    Name (_BBN, 0)

    OperationRegion (CFGS, SystemMemory, BM1000_PCIE2_CFG_BASE, 0x10)
    Field (CFGS, ByteAcc, NoLock, Preserve) {
      Offset (0xE),
      TYPE, 8
    }

    Method (_STA, 0, Serialized) {
      if ((\_SB.STA2 & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                  BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
        Return (0x0)
      }

#ifdef PCIE_SINGLE_DEVICE_BUS_RANGE
      //
      // If we're only exposing one device, then don't bother
      // exposing bridges as we'll never be able to access the EPs
      // hanging off of them.
      //
      if ((TYPE & 0x7F) != 0) {
        Return (0x0)
      }
#endif // PCIE_SINGLE_DEVICE_BUS_RANGE
      Return (0xF)
    }

    Method (_CRS, 0, Serialized) {
      Name (RBUF, ResourceTemplate () {
        BUS_RES
        PROD_MEM_BUF(01)
        PROD_IO_BUF(02)
      })

      RES_BUF_SET(01, BM1000_PCIE2_MMIO32_BASE,  BM1000_PCIE2_MMIO32_SIZE, 0)
      RES_BUF_SET(02, BM1000_PCIE2_PORTIO_MIN,   BM1000_PCIE2_PORTIO_SIZE,
                      BM1000_PCIE2_PORTIO_BASE - BM1000_PCIE2_PORTIO_MIN)

      Return (RBUF)
    }

#ifdef PCIE_NATIVE
    NATIVE_PCIE_OSC
#endif // PCIE_NATIVE
  }
}
