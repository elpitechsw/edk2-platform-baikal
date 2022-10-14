/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <BS1000.h>

#include "AcpiPlatform.h"

#define BUS_RES WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode, 0x0, 0, 0xFF, 0, 0x100)

DefinitionBlock (__FILE__, "SSDT", 2, "BAIKAL", "SSDTPCI0", 1) {
  Scope (_SB_) {
    Device (PCI0) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_P0_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE0_P0_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE0_P0_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE0_P0_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE0_P0_IO_BASE - BAIKAL_ACPI_PCIE0_P0_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE0_P0_DBI_BASE, BS1000_PCIE0_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE0_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 177, 179 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE0_P0_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI1) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_P1_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE0_P1_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE0_P1_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE0_P1_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE0_P1_IO_BASE - BAIKAL_ACPI_PCIE0_P1_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE0_P1_DBI_BASE, BS1000_PCIE0_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE0_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 178, 180 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE0_P1_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI2) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE1_P0_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE1_P0_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE1_P0_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE1_P0_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE1_P0_IO_BASE - BAIKAL_ACPI_PCIE1_P0_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE1_P0_DBI_BASE, BS1000_PCIE1_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE1_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 220, 222 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE1_P0_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI3) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE1_P1_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE1_P1_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE1_P1_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE1_P1_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE1_P1_IO_BASE - BAIKAL_ACPI_PCIE1_P1_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE1_P1_DBI_BASE, BS1000_PCIE1_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE1_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 221, 223 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE1_P1_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI4) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE2_P0_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE2_P0_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE2_P0_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE2_P0_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE2_P0_IO_BASE - BAIKAL_ACPI_PCIE2_P0_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE2_P0_DBI_BASE, BS1000_PCIE2_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE2_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 134, 136 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE2_P0_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI5) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE2_P1_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE2_P1_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE2_P1_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE2_P1_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE2_P1_IO_BASE - BAIKAL_ACPI_PCIE2_P1_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE2_P1_DBI_BASE, BS1000_PCIE2_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE2_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 135, 137 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE2_P1_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI6) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P0_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P0_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE3_P0_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE3_P0_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE3_P0_IO_BASE - BAIKAL_ACPI_PCIE3_P0_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P0_DBI_BASE, BS1000_PCIE3_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 281, 285 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE3_P0_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI7) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P1_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P1_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE3_P1_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE3_P1_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE3_P1_IO_BASE - BAIKAL_ACPI_PCIE3_P1_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P1_DBI_BASE, BS1000_PCIE3_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 282, 286 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE3_P1_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI8) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P2_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P2_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE3_P2_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE3_P2_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE3_P2_IO_BASE - BAIKAL_ACPI_PCIE3_P2_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P2_DBI_BASE, BS1000_PCIE3_P2_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P2_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 283, 287 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE3_P2_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI9) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P3_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P3_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE3_P3_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE3_P3_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE3_P3_IO_BASE - BAIKAL_ACPI_PCIE3_P3_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P3_DBI_BASE, BS1000_PCIE3_P3_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P3_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 284, 288 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE3_P3_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCIA) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE4_P0_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE4_P0_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE4_P0_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE4_P0_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE4_P0_IO_BASE - BAIKAL_ACPI_PCIE4_P0_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P0_DBI_BASE, BS1000_PCIE4_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 350, 354 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE4_P0_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCIB) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE4_P1_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE4_P1_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE4_P1_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE4_P1_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE4_P1_IO_BASE - BAIKAL_ACPI_PCIE4_P1_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P1_DBI_BASE, BS1000_PCIE4_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 351, 355 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE4_P1_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCIC) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE4_P2_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE4_P2_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE4_P2_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE4_P2_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE4_P2_IO_BASE - BAIKAL_ACPI_PCIE4_P2_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P2_DBI_BASE, BS1000_PCIE4_P2_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P2_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 352, 356 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE4_P2_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCID) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE4_P3_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE4_P3_SEGMENT)
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

        RES_BUF_SET(01, BAIKAL_ACPI_PCIE_MEM_MIN, BAIKAL_ACPI_PCIE_MEM_SIZE,
                        BAIKAL_ACPI_PCIE4_P3_MEM_BASE - BAIKAL_ACPI_PCIE_MEM_MIN)
        RES_BUF_SET(02, BAIKAL_ACPI_PCIE4_P3_IO_MIN, BAIKAL_ACPI_PCIE_IO_SIZE,
                        BAIKAL_ACPI_PCIE4_P3_IO_BASE - BAIKAL_ACPI_PCIE4_P3_IO_MIN)

        Return (RBUF)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0) {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P3_DBI_BASE, BS1000_PCIE4_P3_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P3_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 353, 357 }
          })

          RES_BUF_SET(01, BAIKAL_ACPI_PCIE4_P3_CFG_BASE, BAIKAL_ACPI_PCIE_CFG_SIZE, 0)

          Return (RBUF)
        }
      }

      NATIVE_PCIE_OSC
    }
  }
}
