/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Platform/Pcie.h>

#include "AcpiPlatform.h"

#define BUS_RES WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode, 0x0, 0, 0xFF, 0, 0x100)

DefinitionBlock (__FILE__, "SSDT", 2, "BAIKAL", "SSDTPCI0", 1) {
  External (\_SB.CRU0, DeviceObj)
  External (\_SB.GPIO, DeviceObj)
  External (\_SB.GPIO.GPIP, DeviceObj)
  External (\_SB.STA0, FieldUnitObj)
  External (\_SB.STA1, FieldUnitObj)
  External (\_SB.STA2, FieldUnitObj)

  Scope (_SB_) {
    // PCIe0 (x4 #0)
    Device (PCI0) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA, 0, Serialized) {
        if ((\_SB.STA0 & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                         BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
          Return (0x0)
        }

        Return (0xF)
      }

      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          BUS_RES
          PROD_MEM_BUF(01)
          PROD_IO_BUF(02)
        })

        QRES_BUF_SET(01, BM1000_PCIE0_MMIO32_BASE,  BM1000_PCIE0_MMIO32_SIZE, 0)
        QRES_BUF_SET(02, BM1000_PCIE0_PORTIO_MIN,   BM1000_PCIE0_PORTIO_SIZE,
                         BM1000_PCIE0_PORTIO_BASE - BM1000_PCIE0_PORTIO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)
      Name (LCRU, Package () { ^CRU0, 0 })

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BM1000_PCIE0_DBI_BASE, BM1000_PCIE0_DBI_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 458, 461 }
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20) || defined(ELPITECH)
            GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 6 }
#endif
          })

          QRES_BUF_SET(01, BM1000_PCIE0_CFG_BASE, BM1000_PCIE0_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

#ifdef BAIKAL_ACPI_PCIE1_SEGMENT
    // PCIe1 (x4 #1)
    Device (PCI1) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE1_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA, 0, Serialized) {
        if ((\_SB.STA1 & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                         BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
          Return (0x0)
        }

        Return (0xF)
      }

      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          BUS_RES
          PROD_MEM_BUF(01)
          PROD_IO_BUF(02)
        })

        QRES_BUF_SET(01, BM1000_PCIE1_MMIO32_BASE,  BM1000_PCIE1_MMIO32_SIZE, 0)
        QRES_BUF_SET(02, BM1000_PCIE1_PORTIO_MIN,   BM1000_PCIE1_PORTIO_SIZE,
                         BM1000_PCIE1_PORTIO_BASE - BM1000_PCIE1_PORTIO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)
      Name (LCRU, Package () { ^CRU0, One })

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BM1000_PCIE1_DBI_BASE, BM1000_PCIE1_DBI_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 434, 437 }
          })

          QRES_BUF_SET(01, BM1000_PCIE1_CFG_BASE, BM1000_PCIE1_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }
#endif

#ifdef BAIKAL_ACPI_PCIE2_SEGMENT
    // PCIe2 (x8)
    Device (PCI2) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE2_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE2_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA, 0, Serialized) {
        if ((\_SB.STA2 & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                         BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
          Return (0x0)
        }

        Return (0xF)
      }

      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          BUS_RES
          PROD_MEM_BUF(01)
          PROD_IO_BUF(02)
        })

        QRES_BUF_SET(01, BM1000_PCIE2_MMIO32_BASE,  BM1000_PCIE2_MMIO32_SIZE, 0)
        QRES_BUF_SET(02, BM1000_PCIE2_PORTIO_MIN,   BM1000_PCIE2_PORTIO_SIZE,
                         BM1000_PCIE2_PORTIO_BASE - BM1000_PCIE2_PORTIO_MIN)

        Return (RBUF)
      }

      Name (NUML, 8)
      Name (NUMV, 4)
      Name (LCRU, Package () { ^CRU0, 2 })

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BM1000_PCIE2_DBI_BASE, BM1000_PCIE2_DBI_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 410, 413 }
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20) || defined(ELPITECH)
            GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 3 }
#endif
          })

          QRES_BUF_SET(01, BM1000_PCIE2_CFG_BASE, BM1000_PCIE2_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }
#endif
  }
}
