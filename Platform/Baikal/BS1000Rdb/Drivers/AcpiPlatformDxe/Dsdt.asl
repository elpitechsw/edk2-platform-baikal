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

    // QSPI_1
    Device(QSP1) {
      Name(_HID, "HISI0173")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C20000, 0x10000)
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
          SPISerialBusV2(Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 10000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.QSP1")
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

#if 0
    // USB2
    Device(USB2) {
      Name(_CID, "PNP0D10")
      Name(_UID, Zero)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00A10000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 114 }
      })
    }

    // QSPI_2
    Device(QSP2) {
      Name(_HID, "HISI0173")
      Name(_UID, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C30000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 96 }
      })
    }

    // ESPI
    Device(ESPI) {
      Name(_HID, "HISI0173")
      Name(_UID, 2)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00C40000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 102 }
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
#endif

    // UART_S
    Device(COMS) {
      Name(_HID, "APMC0D08")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00E00000, 0x100)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 92 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg-shift", 2 },
          Package() { "reg-io-width", 4 },
          Package() { "clock-frequency", 8000000 }
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
  }
}
