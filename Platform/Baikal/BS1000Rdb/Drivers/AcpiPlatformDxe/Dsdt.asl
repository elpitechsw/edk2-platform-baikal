/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiPlatform.h"

#define BAIKAL_DSDT_CPU_NODE(ClusterId, CpuId) \
  Device(CPU##CpuId) {                         \
    Name(_HID, "ACPI0007")                     \
    Name(_UID, 4 * (ClusterId) + CpuId)        \
  }

#define BAIKAL_DSDT_CLUSTER_NODE(ClusterId) \
  Device(CL##ClusterId) {                   \
    Name(_HID, "ACPI0010")                  \
    Name(_UID, 48 + ClusterId)              \
                                            \
    BAIKAL_DSDT_CPU_NODE(ClusterId, 0)      \
    BAIKAL_DSDT_CPU_NODE(ClusterId, 1)      \
    BAIKAL_DSDT_CPU_NODE(ClusterId, 2)      \
    BAIKAL_DSDT_CPU_NODE(ClusterId, 3)      \
  }

DefinitionBlock("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1) {
  Scope(_SB_) {
    Device(PKG) {
      Name(_HID, "ACPI0010")
      Name(_UID, 60)

      BAIKAL_DSDT_CLUSTER_NODE(0)
      BAIKAL_DSDT_CLUSTER_NODE(1)
      BAIKAL_DSDT_CLUSTER_NODE(2)
      BAIKAL_DSDT_CLUSTER_NODE(3)
      BAIKAL_DSDT_CLUSTER_NODE(4)
      BAIKAL_DSDT_CLUSTER_NODE(5)
      BAIKAL_DSDT_CLUSTER_NODE(6)
      BAIKAL_DSDT_CLUSTER_NODE(7)
      BAIKAL_DSDT_CLUSTER_NODE(8)
      BAIKAL_DSDT_CLUSTER_NODE(9)
      BAIKAL_DSDT_CLUSTER_NODE(10)
      BAIKAL_DSDT_CLUSTER_NODE(11)
    }

    Device(CLK0) {
      Name(_HID, "BKLE0001")
      Name(_UID, Zero)
      /* CMU addr, clock name */
      Name(PROP, Package() {
        0x00410000, "cmu_sc_1"
      })
      /* Device reference, clock name, clock id, con_id */
      Name(CLKS, Package() {
        ^GMC0, "gmac1_apb", 9, "stmmaceth",
        ^GMC0, "gmac1_axi", 10, "axi_clk",
        ^GMC1, "gmac2_apb", 11, "stmmaceth",
        ^GMC1, "gmac2_axi", 12, "axi_clk"
      })
    }

    Device(CLK1) {
      Name(_HID, "BKLE0001")
      Name(_UID, One)
      /* CMU addr, clock name */
      Name(PROP, Package() {
        0x00420000, "cmu_sc_2"
      })
      /* Device reference, clock name, clock id, con_id */
      Name(CLKS, Package() {
        ^GMC0, "gmac1_ptp", 1, "ptp_ref",
        ^GMC0, "gmac1_txx2", 2, "tx2_clk",
        ^GMC1, "gmac2_ptp", 3, "ptp_ref",
        ^GMC1, "gmac2_txx2", 4, "tx2_clk"
      })
    }

    // PVT_CLUSTER0
    Device(PVC0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x04030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x04030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER1
    Device(PVC1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x08030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x08030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER2
    Device(PVC2) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x0C030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x0C030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER3
    Device(PVC3) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x10030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x10030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER4
    Device(PVC4) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x14030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x14030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER5
    Device(PVC5) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x18030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x18030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER6
    Device(PVC6) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x1C030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x1C030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER7
    Device(PVC7) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x20030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER8
    Device(PVC8) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x24030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x24030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER9
    Device(PVC9) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x28030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x28030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER10
    Device(PVCA) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x2C030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2C030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER11
    Device(PVCB) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x30030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30030000, 0x1000)
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE0
    Device(PVP0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x38030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x38030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 195 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE1
    Device(PVP1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x3C030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x3C030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 238 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE2
    Device(PVP2) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x44030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x44030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 152 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE3
    Device(PVP3) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x48030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x48030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 307 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE4
    Device(PVP4) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x4C030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x4C030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 376 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR0
    Device(PVD0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x50030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x50030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 395 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR1
    Device(PVD1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x54030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x54030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 412 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR2
    Device(PVD2) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x58030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x58030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 429 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR3
    Device(PVD3) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x60030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x60030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 446 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR4
    Device(PVD4) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x64030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x64030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 463 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR5
    Device(PVD5) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x68030000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x68030000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 480 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // USB OHCI
    Device(USB1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x00A00000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00A00000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 110 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "generic-ohci" }
        }
      })
    }

    // USB2
    Device(USB2) {
      Name(_CID, "PNP0D20")
      Name(_UID, Zero)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00A10000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 114 }
      })
    }

    // QSPI_1
    Device(QSP1) {
      Name(_HID, "HISI0173")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C20000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 95 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "num-cs", 4 }
        }
      })

      Device(PR00) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x00C20000)
        Name(_ADR, Zero)
        Name(_CRS, ResourceTemplate() {
          SPISerialBusV2(Zero, PolarityHigh, FourWireMode, 8, ControllerInitiated, 10000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.QSP1")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "jedec,spi-nor" },
            Package() { "reg", Zero }
          }
        })
      }
    }

    // QSPI_2
    Device(QSP2) {
      Name(_HID, "HISI0173")
      Name(_UID, One)
      Method (_STA, 0, Serialized) {
        Return (\_SB.MUX1.STA1)
      }
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C30000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 96 }
      })
    }

    // ESPI
    Device(ESPI) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x00C40000)
      Method (_STA, 0, Serialized) {
        Return (\_SB.MUX2.STA1)
      }
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C40000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 102 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-espi" }
        }
      })
    }

    // GPIO32
    Device(GP32) {
      Name(_HID, "APMC0D07")
      Name(_UID, 0x00C50000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C50000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 86 }
      })

      // GPIO port
      Device(GPIP) {
        Name(_ADR, Zero)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "reg", Zero },
            Package() { "snps,nr-gpios", 32 }
          }
        })
      }
    }

    // GPIO16
    Device(GP16) {
      Name(_HID, "APMC0D07")
      Name(_UID, 0x00C60000)
      Method (_STA, 0, Serialized) {
        Return (\_SB.MUX2.STA0)
      }
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C60000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 83 }
      })

      // GPIO port
      Device(GPIP) {
        Name(_ADR, Zero)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "reg", Zero },
            Package() { "snps,nr-gpios", 16 }
          }
        })
      }
    }

    // GPIO8_1
    Device(GP81) {
      Name(_HID, "APMC0D07")
      Name(_UID, 0x00C70000)
      Method (_STA, 0, Serialized) {
        Return (\_SB.MUX0.STA0)
      }
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C70000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 84 }
      })

      // GPIO port
      Device(GPIP) {
        Name(_ADR, Zero)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "reg", Zero },
            Package() { "snps,nr-gpios", 8 }
          }
        })
      }
    }

    // GPIO8_2
    Device(GP82) {
      Name(_HID, "APMC0D07")
      Name(_UID, 0x00C80000)
      Method (_STA, 0, Serialized) {
        Return (\_SB.MUX1.STA0)
      }
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C80000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 85 }
      })

      // GPIO port
      Device(GPIP) {
        Name(_ADR, Zero)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "reg", Zero },
            Package() { "snps,nr-gpios", 8 }
          }
        })
      }
    }

    // SMBUS_I2C2
    Device(I2C2) {
      Name(_HID, "APMC0D0F")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C90000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 97 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "i2c-sda-hold-time-ns", 500 },
          Package() { "clock-frequency", 400000 }
        }
      })
    }

    // SMBUS_I2C3
    Device(I2C3) {
      Name(_HID, "APMC0D0F")
      Name(_UID, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00CA0000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 98 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "i2c-sda-hold-time-ns", 500 },
          Package() { "clock-frequency", 400000 }
        }
      })
    }

    // SMBUS_I2C4
    Device(I2C4) {
      Name(_HID, "APMC0D0F")
      Name(_UID, 2)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00CB0000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 99 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "i2c-sda-hold-time-ns", 500 },
          Package() { "clock-frequency", 400000 }
        }
      })
    }

    // SMBUS_I2C5
    Device(I2C5) {
      Name(_HID, "APMC0D0F")
      Name(_UID, 3)
      Method (_STA, 0, Serialized) {
        Return (\_SB.MUX0.STA1)
      }
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00CC0000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 100 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "i2c-sda-hold-time-ns", 500 },
          Package() { "clock-frequency", 400000 }
        }
      })
    }

    // SMBUS_I2C6
    Device(I2C6) {
      Name(_HID, "APMC0D0F")
      Name(_UID, 4)
      Method (_STA, 0, Serialized) {
        Return (\_SB.MUX0.STA1)
      }
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00CD0000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 101 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "i2c-sda-hold-time-ns", 500 },
          Package() { "clock-frequency", 400000 }
        }
      })
    }

    // UART_S
    Device(COMS) {
      Name(_HID, "HISI0031")
      Name(_UID, Zero)
      Method (_STA, 0, Serialized) {
        Return (\_SB.MUX0.STA1)
      }
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00E00000, 0x100)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 92 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg-shift", 2 },
          Package() { "reg-io-width", 4 },
          Package() { "clock-frequency", 7273800 }
        }
      })
    }

    Device(COM0) {
      Name(_HID, "ARMH0011")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C00000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 93 }
      })
    }

    Device(COM1) {
      Name(_HID, "ARMH0011")
      Name(_UID, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C10000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 94 }
      })
    }

    // GMAC0
    Device(GMC0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x00A20000)
      Name(_CCA, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00A20000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 108 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-gmac" },
          Package() { "reg", One },
          Package() { "phy-mode", "rgmii-rxid" },
        }
      })

      Device(GPHY) {
        Name(_ADR, Zero)
      }
    }

    // GMAC1
    Device(GMC1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x00A30000)
      Name(_CCA, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00A30000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 109 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bs1000-gmac" },
          Package() { "reg", One },
          Package() { "phy-mode", "rgmii-rxid" },
        }
      })

      Device(GPHY) {
        Name(_ADR, Zero)
      }
    }

    Device(MUXC) {
      Name(_HID, "BKLE0002")
      Name(_UID, Zero)
    }

    Device(MUX0) {
      Name(_HID, "BKLE0003")
      Name(_UID, Zero)
      Name(MUX, Package() { ^MUXC, 0 })
      Name(DEV0, Package() { ^GP81 })
      Name(DEV1, Package() { ^COMS, I2C5, I2C6 })
      Name(STA0, Zero)
      Name(STA1, Zero)
      Method (INIT, 1, Serialized) {
        If (LEqual (Arg0, Zero))
        {
          Store (0xF, STA0)
        }
        Else
        {
          Store (0xF, STA1)
        }
      }
    }

    Device(MUX1) {
      Name(_HID, "BKLE0003")
      Name(_UID, One)
      Name(MUX, Package() { ^MUXC, 1 })
      Name(DEV0, Package() { ^GP82 })
      Name(DEV1, Package() { ^QSP2 })
      Name(STA0, Zero)
      Name(STA1, Zero)
      Method (INIT, 1, Serialized) {
        If (LEqual (Arg0, Zero))
        {
          Store (0xF, STA0)
        }
        Else
        {
          Store (0xF, STA1)
        }
      }
    }

    Device(MUX2) {
      Name(_HID, "BKLE0003")
      Name(_UID, 2)
      Name(MUX, Package() { ^MUXC, 2 })
      Name(DEV0, Package() { ^GP16 })
      Name(DEV1, Package() { ^ESPI })
      Name(STA0, Zero)
      Name(STA1, Zero)
      Method (INIT, 1, Serialized) {
        If (LEqual (Arg0, Zero))
        {
          Store (0xF, STA0)
        }
        Else
        {
          Store (0xF, STA1)
        }
      }
    }
  }
}
