/** @file
  Copyright (c) 2020 - 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiPlatform.h"

#define BAIKAL_ACPI_PROCESSOR_LPI                         \
  Name(_LPI, Package() {                                  \
    0,                                                    \
    0,                                                    \
    2,                                                    \
    Package() {                                           \
      1,                                                  \
      1,                                                  \
      1,                                                  \
      0,                                                  \
      0,                                                  \
      0,                                                  \
      ResourceTemplate () {                               \
        Register (FFixedHW, 0x20, 0x00, 0xFFFFFFFF, 0x03) \
      },                                                  \
      ResourceTemplate() {                                \
        Register (SystemMemory, 0, 0, 0, 0)               \
      },                                                  \
      ResourceTemplate() {                                \
        Register (SystemMemory, 0, 0, 0, 0)               \
      },                                                  \
      "WFI"                                               \
    },                                                    \
    Package() {                                           \
      2000,                                               \
      1500,                                               \
      1,                                                  \
      1,                                                  \
      0,                                                  \
      1,                                                  \
      ResourceTemplate () {                               \
        Register (FFixedHW, 0x20, 0x00, 0x00000001, 0x03) \
      },                                                  \
      ResourceTemplate() {                                \
        Register (SystemMemory, 0, 0, 0, 0)               \
      },                                                  \
      ResourceTemplate() {                                \
        Register (SystemMemory, 0, 0, 0, 0)               \
      },                                                  \
      "cpu-sleep"                                         \
    }                                                     \
  })

DefinitionBlock("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1) {
  Scope(_SB_) {
    Method (_OSC, 4) {
      CreateDWordField (Arg3, Zero, CDW1)

      /* Check for proper UUID */
      If (LEqual (Arg0, ToUUID ("0811B06E-4A27-44F9-8D60-3CBBC22E7B48"))) {
        /* Allow everything by default */

        /* Unknown revision */
        If (LNotEqual (Arg1, One)) {
          Or (CDW1, 0x08, CDW1)
        }
      } Else {
        /* Unrecognized UUID */
        Or (CDW1, 4, CDW1)
      }

      Return (Arg3)
    }

    Device(PKG) {
      Name(_HID, "ACPI0010")
      Name(_UID, 12)

      // Cluster 0
      Device(CLU0) {
        Name(_HID, "ACPI0010")
        Name(_UID, 8)

        // Cpu 0
        Device(CPU0) {
          Name(_HID, "ACPI0007")
          Name(_UID, Zero)

          BAIKAL_ACPI_PROCESSOR_LPI
        }

        // Cpu 1
        Device(CPU1) {
          Name(_HID, "ACPI0007")
          Name(_UID, One)

          BAIKAL_ACPI_PROCESSOR_LPI
        }
      }

      // Cluster 1
      Device(CLU1) {
        Name(_HID, "ACPI0010")
        Name(_UID, 9)

        // Cpu 2
        Device(CPU0) {
          Name(_HID, "ACPI0007")
          Name(_UID, 2)

          BAIKAL_ACPI_PROCESSOR_LPI
        }

        // Cpu 3
        Device(CPU1) {
          Name(_HID, "ACPI0007")
          Name(_UID, 3)

          BAIKAL_ACPI_PROCESSOR_LPI
        }
      }

      // Cluster 2
      Device(CLU2) {
        Name(_HID, "ACPI0010")
        Name(_UID, 10)

        // Cpu 4
        Device(CPU0) {
          Name(_HID, "ACPI0007")
          Name(_UID, 4)

          BAIKAL_ACPI_PROCESSOR_LPI
        }

        // Cpu 5
        Device(CPU1) {
          Name(_HID, "ACPI0007")
          Name(_UID, 5)

          BAIKAL_ACPI_PROCESSOR_LPI
        }
      }

      // Cluster 3
      Device(CLU3) {
        Name(_HID, "ACPI0010")
        Name(_UID, 11)

        // Cpu 6
        Device(CPU0) {
          Name(_HID, "ACPI0007")
          Name(_UID, 6)

          BAIKAL_ACPI_PROCESSOR_LPI
        }

        // Cpu 7
        Device(CPU1) {
          Name(_HID, "ACPI0007")
          Name(_UID, 7)

          BAIKAL_ACPI_PROCESSOR_LPI
        }
      }
    }

    Device(CLK0) {
      Name(_HID, "BKLE0001")
      Name(_UID, Zero)
      /* CMU id, clock name, frequency, is_osc27 */
      Name(PROP, Package() {
        0x20000000, "baikal_avlsp_cmu", 1200000000, Zero
      })
      /* Device reference, clock name, clock id, con_id */
      Name(CLKS, Package() {
        ^SPI0, "spi", 4, Zero,
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
        ^ESPI, "espi", 5, Zero,
#endif
        ^I2C0, "i2c1", 6, Zero,
        ^I2C1, "i2c2", 7, Zero,
        ^SMB0, "smbus1", 13, Zero,
        ^SMB1, "smbus2", 14, Zero,
        ^MMC0, "mshc_ahb", 18, "bus",
        ^MMC0, "mshc_tx_x2", 19, "core",
      })
    }

    Device(CLK1) {
      Name(_HID, "BKLE0001")
      Name(_UID, One)
      /* CMU id, clock name, frequency, is_osc27 */
      Name(PROP, Package() {
        0x30000000, "baikal_xgb_cmu0", 1250000000, Zero
      })
      /* Device reference, clock name, clock id, con_id */
      Name(CLKS, Package() {
        ^GMC0, "gmac0_tx2", 10, "tx2_clk",
        ^GMC1, "gmac1_tx2", 13, "tx2_clk",
      })
    }

    // PCIe LCRU
    Device (CRU0) {
      Name (_ADR, 0x02000000)
      Name (_UID, 0x02000000)
      Name (_CRS, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0x02000000, 0x80000)
      })
    }

    // PVT2
    Device(PVT2) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x0A200000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x0A200000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 155 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-pvt" },
          Package() { "pvt_id", 2 }
        }
      })
    }

    // PVT1
    Device(PVT1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x0C200000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x0C200000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 153 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-pvt" },
          Package() { "pvt_id", One }
        }
      })
    }

    // DDR0
    Device(DDR0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x0E200000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x0E200000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 161, 162, 163, 164, 165, 166 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-edac-mc" }
        }
      })
    }

    // DDR1
    Device(DDR1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x22200000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x22200000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 171, 172, 173, 174, 175, 176 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-edac-mc" }
        }
      })
    }

    // GPIO
    Device(GPIO) {
      Name(_HID, "APMC0D07")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20200000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 131 }
      })

      // GPIO port
      Device(GPIP) {
        Name(_ADR, Zero)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "reg", Zero },
            Package() { "snps,nr-gpios", 32 },
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20) || defined (ELPITECH)
            Package() { "line-name", "pcie-x8-clock" },
            Package() { "gpio-hog", One },
            Package() { "gpios", Package() { One, One } },
            Package() { "output-high", One }
#endif
          }
        })
      }
    }

    // SPI
    Device(SPI0) {
      Name(_HID, "HISI0173")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20210000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 132 }
        GpioIo(Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 24, 25, 26, 27 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "num-cs", 4 },
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
          Package() { "cs-gpios", Package() {
            ^SPI0, Zero, Zero, One,
            ^SPI0, Zero, One, One,
            ^SPI0, Zero, 2, One,
            ^SPI0, Zero, 3, One
          }}
        }
      })

      Device(PR00) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20210000)
        Name(_ADR, Zero)
        Name(_CRS, ResourceTemplate() {
          SPISerialBusV2(Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 12500000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.SPI0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", Package() { "micron,n25q256a", "jedec,spi-nor" } },
            Package() { "reg", Zero }
          }
        })
      }
#elif defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20) || defined (ELPITECH)
          Package() { "cs-gpios", Zero }
        }
      })
#else
        }
      })
#endif
    }

    // UART1
    Device(COM0) {
      Name(_HID, "HISI0031")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20230000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 133 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg-shift", 2 },
          Package() { "reg-io-width", 4 },
          Package() { "clock-frequency", 7361963 }
        }
      })
    }

    // UART2
    Device(COM1) {
      Name(_HID, "HISI0031")
      Name(_UID, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20240000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 134 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg-shift", 2 },
          Package() { "reg-io-width", 4 },
          Package() { "clock-frequency", 7361963 }
        }
      })
    }

    // I2C0
    Device(I2C0) {
      Name(_HID, "APMC0D0F")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20250000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 140 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "i2c-sda-hold-time-ns", 500 },
          Package() { "clock-frequency", 400000 }
        }
      })

#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
      Device(PR08) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250008)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x08, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "tp,mitx2-bmc" }
          }
        })
      }

      Device(PR50) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250050)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x50, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "nxp,pca9670" }
          }
        })
      }

      Device(PR51) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250051)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x51, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", Package() { "nxp,pcf2129", "nxp,pcf2127" } }
          }
        })
      }

      Device(PR52) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250052)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x52, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "tp,bm_mitx_hwmon" }
          }
        })
      }

      Device(PR53) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250053)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x53, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "atmel,24c32" },
            Package() { "pagesize", 32 }
          }
        })
      }

      Device(PR54) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250054)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x54, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "tp,tp_serio" }
          }
        })
      }
#elif defined(BAIKAL_DBM10)
      Device(PR56) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250056)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x56, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "abracon,abeoz9" }
          }
        })
      }
#endif
    }

    // I2C1
    Device(I2C1) {
      Name(_HID, "APMC0D0F")
      Name(_UID, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20260000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 141 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "i2c-sda-hold-time-ns", 500 },
          Package() { "clock-frequency", 400000 }
        }
      })

#ifdef BAIKAL_DBM20
      Device(PR56) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20260056)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x56, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "abracon,abeoz9" }
          }
        })
      }
#endif
    }

    // SMBUS1
    Device(SMB0) {
      Name(_HID, "PRP0001")
      Name(_CID, "PNP0500")
      Name(_UID, 0x20270000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20270000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 142 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-smbus" },
          Package() { "clock-frequency", 100000 }
        }
      })
      Name(CLK, 50000000)
    }

    // SMBUS2
    Device(SMB1) {
      Name(_HID, "PRP0001")
      Name(_CID, "PNP0500")
      Name(_UID, 0x20280000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20280000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 143 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-smbus" },
          Package() { "clock-frequency", 100000 }
        }
      })
      Name(CLK, 50000000)
    }

#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
    // ESPI
    Device(ESPI) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x202A0000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x202A0000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 135 }
        GpioIo(Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 28 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-espi" },
          Package() { "gpios", Package() { ^ESPI, Zero, Zero, One } } // as in dts todo: get real gpio
        }
      })

      Device(PR00) {
        Name(_HID, "SPT0001")
        Name(_UID, 0x202A0001)
        Name(_CRS, ResourceTemplate() {
          SPISerialBusV2(Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 12000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.ESPI")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "rohm,dh2228fv" }
          }
        })
      }
    }
#endif

    // SD/eMMC
    Device(MMC0) {
      Name(_HID, "PRP0001")
      Name(_CID, "PNP0D40")
      Name(_UID, 0x202E0000)
      Name(_CCA, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x202E0000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 83, 84 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "snps,dwcmshc-sdhci" },
          Package() { "disable-wp", 1 },
          Package() { "bus-width", 4 },
          Package() { "max-clock", 25000000 }
        }
      })
    }

    // PVT3
    Device(PVT3) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x26200000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x26200000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 157 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-pvt" },
          Package() { "pvt_id", 3 }
        }
      })
    }

    // PVT0
    Device(PVT0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x28200000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x28200000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 151 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-pvt" },
          Package() { "pvt_id", Zero }
        }
      })
    }

    // PVT_MALI
    Device(PVTM) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x2A060000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2A060000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 253 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-pvt" },
          Package() { "pvt_id", 4 }
        }
      })
    }

    // USB2
    Device(USB2) {
      Name(_HID, "808622B7")
      Name(_CID, "PNP0D10")
      Name(_UID, Zero)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2C400000, 0x100000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 267, 268, 277 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "dr_mode", "host" },
          Package() { "maximum-speed", "high-speed" }
        }
      })

      Device(RHUB) {
        Name(_ADR, Zero)

        Device(PRT1) {
          Name(_ADR, One)
          Name(_UPC, Package() { 0xFF, Zero, Zero, Zero })
          Name(_PLD, Package() {
            Buffer() {
              0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0xFF, 0xFF, 0xFF, 0xFF
            }
          })
        }

        Device(PRT2) {
          Name(_ADR, 2)
          Name(_UPC, Package() { 0xFF, Zero, Zero, Zero })
          Name(_PLD, Package() {
            Buffer() {
              0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0xFF, 0xFF, 0xFF, 0xFF
            }
          })
        }
      }
    }

    // USB3
    Device(USB3) {
      Name(_HID, "808622B7")
      Name(_CID, "PNP0D10")
      Name(_UID, One)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2C500000, 0x100000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {
          269, 270, 271, 272, 273, 274, 275, 276, 278
        }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "dr_mode", "host" }
        }
      })

      Device(RHUB) {
        Name(_ADR, Zero)

        Device(PRT1) {
          Name(_ADR, One)
          Name(_UPC, Package() { 0xFF, Zero, Zero, Zero })
          Name(_PLD, Package() {
            Buffer() {
              0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0xFF, 0xFF, 0xFF, 0xFF
            }
          })
        }

        Device(PRT2) {
          Name(_ADR, 2)
          Name(_UPC, Package() { 0xFF, Zero, Zero, Zero })
          Name(_PLD, Package() {
            Buffer() {
              0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0xFF, 0xFF, 0xFF, 0xFF
            }
          })
        }

        Device(PRT3) {
          Name(_ADR, 3)
          Name(_UPC, Package() { 0xFF, 0x03, Zero, Zero })
          Name(_PLD, Package() {
            Buffer() {
              0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0xFF, 0xFF, 0xFF, 0xFF
            }
          })
        }

        Device(PRT4) {
          Name(_ADR, 4)
          Name(_UPC, Package() { 0xFF, 0x03, Zero, Zero })
          Name(_PLD, Package() {
            Buffer() {
              0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0xFF, 0xFF, 0xFF, 0xFF
            }
          })
        }
      }
    }

    // SATA0
    Device(SAT0) {
      Name(_HID, "PRP0001")
      Name(_CID, "PNP0600")
      Name(_CLS, Package() { One, 0x06, One })
      Name(_UID, 0x2C600000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2C600000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 265 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", Package() { "snps,dwc-ahci", "generic-ahci" } }
        }
      })
    }

    // SATA1
    Device(SAT1) {
      Name(_HID, "PRP0001")
      Name(_CID, "PNP0600")
      Name(_CLS, Package() { One, 0x06, One })
      Name(_UID, 0x2C610000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2C610000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 266 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", Package() { "snps,dwc-ahci", "generic-ahci" } }
        }
      })
    }

#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
    // Internal MDIO
    Device(MDIO) {
      Name(_HID, "PRP0001")
      Name(_UID, 200)
      Name(_CRS, ResourceTemplate() {
        GpioIo(Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 30, 29 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,mdio-gpio" },
          Package() { "bus-id", Zero },
          Package() { "mdc-gpio", Package() {
            ^MDIO, Zero, Zero, Zero
          }},
          Package() { "mdio-gpio", Package() {
            ^MDIO, Zero, One, Zero
          }}
        }
      })

      Device(PR0C) {
        Name(_ADR, 0x0C)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "reg", 0x0C },
            Package() { "mv,line-mode", "KR" },
            Package() { "mv,host-mode", "KX4" }
          }
        })
      }

      Device(PR0E) {
        Name(_ADR, 0x0E)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "reg", 0x0E },
            Package() { "mv,line-mode", "KR" },
            Package() { "mv,host-mode", "KX4" }
          }
        })
      }
    }

    // XGMAC0
    Device(XGM0) {
      Name(_HID, "AMDI8001")
      Name(_UID, Zero)
      Name(_CCA, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30200000, 0x10000)
        Memory32Fixed(ReadWrite, 0x30210000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {
          325, 326, 327, 328, 329, 330, 331, 332,
          333, 334, 335, 336, 337, 338, 339, 340,
          341
        }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "amd,dma-freq", 156250000 },
          Package() { "amd,ptp-freq", 156250000 },
          Package() { "amd,per-channel-interrupt", One },
          Package() { "amd,speed-set", Zero },
          Package() { "phy-mode", "xgmii" },
          Package() { "mac-address", Package() { 0x4C, 0xA5, 0x15, 0x00, 0x00, 0x00 } },
          Package() { "be,pcs-mode", "KX4" },
          Package() { "ext-phy-handle", ^MDIO.PR0C }
        }
      })
    }

    // XGMAC1
    Device(XGM1) {
      Name(_HID, "AMDI8001")
      Name(_UID, One)
      Name(_CCA, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30220000, 0x10000)
        Memory32Fixed(ReadWrite, 0x30230000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) {
          342, 343, 344, 345, 346, 347, 348, 349,
          350, 351, 352, 353, 354, 355, 356, 357,
          358
        }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "amd,dma-freq", 156250000 },
          Package() { "amd,ptp-freq", 156250000 },
          Package() { "amd,per-channel-interrupt", One },
          Package() { "amd,speed-set", Zero },
          Package() { "phy-mode", "xgmii" },
          Package() { "mac-address", Package() { 0x4C, 0xA5, 0x15, 0x00, 0x00, 0x01 } },
          Package() { "be,pcs-mode", "KX4" },
          Package() { "ext-phy-handle", ^MDIO.PR0E }
        }
      })
    }
#endif

    // GMAC0
    Device(GMC0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x30240000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30240000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 323 }
#ifdef BAIKAL_MBM20 || defined (ELPITECH)
        GpioIo(Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 19 }
#endif
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-gmac" },
          Package() { "max-speed", 1000 },
          Package() { "reg", 3 },
          Package() { "phy-mode", "rgmii-id" },
          Package() { "stmmac-clk", 50000000 },
          Package() { "snps,fixed-burst", One },
          Package() { "snps,txpbl", 4 },
          Package() { "snps,rxpbl", 4 },
          Package() { "snps,blen", Package() { Zero, Zero, Zero, Zero, Zero, Zero, 4 } },
#ifdef BAIKAL_MBM20 || defined (ELPITECH)
          Package() { "snps,reset-gpios", Package() {
            ^GMC0, Zero, Zero, One,
          }},
          Package() { "snps,reset-delays-us", Package() { Zero, 10000, 50000 } }
#endif
        }
      })

      Device(GPHY) {
        Name(_ADR, Zero)
      }
    }

    // GMAC1
    Device(GMC1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x30250000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30250000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 324 }
#ifdef BAIKAL_MBM20 || defined (ELPITECH)
        GpioIo(Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 20 }
#endif
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,bm1000-gmac" },
          Package() { "max-speed", 1000 },
          Package() { "reg", 3 },
          Package() { "phy-mode", "rgmii-id" },
          Package() { "stmmac-clk", 50000000 },
          Package() { "snps,fixed-burst", One },
          Package() { "snps,txpbl", 4 },
          Package() { "snps,rxpbl", 4 },
          Package() { "snps,blen", Package() { Zero, Zero, Zero, Zero, Zero, Zero, 4 } },
#ifdef BAIKAL_MBM20 || defined (ELPITECH)
          Package() { "snps,reset-gpios", Package() {
            ^GMC1, Zero, Zero, One,
          }},
          Package() { "snps,reset-delays-us", Package() { Zero, 10000, 50000 } }
#endif
        }
      })

      Device(GPHY) {
        Name(_ADR, Zero)
      }
    }

#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
    Device(LEDS) {
      Name(_HID, "PRP0001")
      Name(_UID, 600)
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "gpio-leds" }
        }
      })

      Device(LED0) {
        Name(_ADR, Zero)
        Name(_CRS, ResourceTemplate() {
          GpioIo(Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 8 }
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "default-state", "keep" },
            Package() { "label", "led0" },
            Package() { "gpios", Package() { ^LED0, Zero, Zero, Zero } }
          }
        })
      }
    }
#endif
  }
}
