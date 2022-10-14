/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <BM1000.h>

#include "AcpiPlatform.h"

#define BUS_RES WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode, 0x0, 0, 0xFF, 0, 0x100)

DefinitionBlock (__FILE__, "SSDT", 2, "BAIKAL", "SSDTPCI0", 1) {
  External (\_SB.CRU0, DeviceObj)
  External (\_SB.GPIO, DeviceObj)
  External (\_SB.GPIO.GPIP, DeviceObj)

  Scope (_SB_) {
    // PCIe0 (x4 #0)
    Device (PCI0) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE0_SEGMENT)
      Name (_BBN, Zero)

      Name (_PRT, Package() {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          BUS_RES
          PROD_MEM_BUF(01)
          PROD_IO_BUF(02)
        })

        QRES_BUF_SET(01, BAIKAL_ACPI_PCIE0_MEM_MIN, BAIKAL_ACPI_PCIE0_MEM_SIZE,
                         BAIKAL_ACPI_PCIE0_MEM_BASE - BAIKAL_ACPI_PCIE0_MEM_MIN)
        QRES_BUF_SET(02, BAIKAL_ACPI_PCIE_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                         BAIKAL_ACPI_PCIE0_IO_BASE - BAIKAL_ACPI_PCIE_IO_MIN)

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
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE0_DBI_BASE, BAIKAL_ACPI_PCIE_DBI_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 458, 461 }
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20) || defined(ELPITECH)
            GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 6 }
#endif
          })

          QRES_BUF_SET(01, BAIKAL_ACPI_PCIE0_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

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

      Name (_PRT, Package() {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          BUS_RES
          PROD_MEM_BUF(01)
          PROD_IO_BUF(02)
        })

        QRES_BUF_SET(01, BAIKAL_ACPI_PCIE1_MEM_MIN, BAIKAL_ACPI_PCIE1_MEM_SIZE,
                         BAIKAL_ACPI_PCIE1_MEM_BASE - BAIKAL_ACPI_PCIE1_MEM_MIN)
        QRES_BUF_SET(02, BAIKAL_ACPI_PCIE_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                         BAIKAL_ACPI_PCIE1_IO_BASE - BAIKAL_ACPI_PCIE_IO_MIN)

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
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE1_DBI_BASE, BAIKAL_ACPI_PCIE_DBI_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 434, 437 }
          })

          QRES_BUF_SET(01, BAIKAL_ACPI_PCIE1_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

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

      Name (_PRT, Package() {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          BUS_RES
          PROD_MEM_BUF(01)
          PROD_IO_BUF(02)
        })

        QRES_BUF_SET(01, BAIKAL_ACPI_PCIE2_MEM_MIN, BAIKAL_ACPI_PCIE2_MEM_SIZE,
                         BAIKAL_ACPI_PCIE2_MEM_BASE - BAIKAL_ACPI_PCIE2_MEM_MIN)
        QRES_BUF_SET(02, BAIKAL_ACPI_PCIE_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                         BAIKAL_ACPI_PCIE2_IO_BASE - BAIKAL_ACPI_PCIE_IO_MIN)

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
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE2_DBI_BASE, BAIKAL_ACPI_PCIE_DBI_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 410, 413 }
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20) || defined(ELPITECH)
            GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 3 }
#endif
          })

          QRES_BUF_SET(01, BAIKAL_ACPI_PCIE2_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }
#endif
  }
}
