/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <BM1000.h>
#include "AcpiPlatform.h"

DefinitionBlock("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1) {
  Scope(_SB_) {
    OperationRegion (LCRU, SystemMemory, BM1000_MMPCIE_GPR_BASE, 0x1000)
    Field (LCRU, DWordAcc, NoLock, Preserve) {
      Offset (0x04),
      STA0, 32,
      Offset (0x24),
      STA1, 32,
      Offset (0x44),
      STA2, 32
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
        }

        // Cpu 1
        Device(CPU1) {
          Name(_HID, "ACPI0007")
          Name(_UID, One)
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
        }

        // Cpu 3
        Device(CPU1) {
          Name(_HID, "ACPI0007")
          Name(_UID, 3)
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
        }

        // Cpu 5
        Device(CPU1) {
          Name(_HID, "ACPI0007")
          Name(_UID, 5)
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
        }

        // Cpu 7
        Device(CPU1) {
          Name(_HID, "ACPI0007")
          Name(_UID, 7)
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
#ifdef BAIKAL_DBM
        ^ESPI, "espi", 5, Zero,
#endif
        ^I2C0, "i2c1", 6, Zero,
        ^I2C1, "i2c2", 7, Zero,
        ^SMB0, "smbus1", 13, Zero,
        ^SMB1, "smbus2", 14, Zero,
#ifdef BAIKAL_DBM
        ^HDA0, "hda_sys_clk", 15, "hda_sys_clk",
        ^HDA0, "hda_clk48", 16, "hda_clk48",
#endif
        ^MMC0, "mshc_ahb", 18, "bus",
        ^MMC0, "mshc_tx_x2", 19, "core",
#if 0
#if defined (BAIKAL_DBM) || defined (BAIKAL_MBM20) // remove MBM20 before release
        ^VDU0, "lvds", 26, "pclk"
#endif
#endif
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
#if 0
        ^HDMI, "csr50mhz", 0, "iahb",
#endif
#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
        ^GMC0, "gmac0_tx2", 10, "tx2_clk",
        ^GMC1, "gmac1_tx2", 13, "tx2_clk",
#endif
#if 0
        ^HDMI, "isfr", 17, "isfr"
#endif
      })
    }

    Device(CLK2) {
      Name(_HID, "BKLE0001")
      Name(_UID, 2)
      /* CMU id, clock name, frequency, is_osc27 */
      Name(PROP, Package() {
        0x30010000, "baikal_xgb_cmu1", 25250000, One
      })
      /* Device reference, clock name, clock id, con_id */
      Name(CLKS, Package() {
#if 0
        ^VDU1, "hdmi", Zero, "pclk"
#endif
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
          Package() { "compatible", "baikal,pvt" },
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
          Package() { "compatible", "baikal,pvt" },
          Package() { "pvt_id", One }
        }
      })
    }

/*******************
 * Not used device *
 *******************/
#if 0
    // DDR1
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
          Package() { "compatible", Package() { "be,emc", "be,memory-controller" } }
        }
      })
    }
#endif

    // GPIO
    Device(GPIO) {
      Name(_HID, "APMC0D07")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20200000, 0x10000)
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
#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
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
#ifdef BAIKAL_DBM
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
#elif defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
          Package() { "cs-gpios", Zero }
        }
      })
#else
        }
      })
#endif
    }

#if 0
    // I2S
    Device(I2S0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x20220000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20220000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 136, 137, 138, 139 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "snps,designware-i2s" }
        }
      })
    }
#endif

    // UART1
    Device(COM0) {
      Name(_HID, "APMC0D08")
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
      Name(_HID, "APMC0D08")
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

#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
      Device(PR08) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250008)
        Name(_ADR, 0x08)
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

      Device(PR1A) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x2025001A)
        Name(_ADR, 0x1A)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x1A, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "nuvoton,nau8822" }
          }
        })
      }

      Device(PR50) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250050)
        Name(_ADR, 0x50)
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
        Name(_ADR, 0x51)
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

#if 0
      Device(PR52) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250052)
        Name(_ADR, 0x52)
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
#endif

      Device(PR53) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250053)
        Name(_ADR, 0x53)
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

#if 0
      Device(PR54) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250054)
        Name(_ADR, 0x54)
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
#endif
#elif defined(BAIKAL_DBM)
#if 0
      Device(PR18) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250018)
        Name(_ADR, 0x18)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x18, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
        })
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "ti,tlv320aic3x" }
          }
        })
      }
#endif

      Device(PR56) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x20250056)
        Name(_ADR, 0x56)
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

/**************************
 * Baikal driver need fix *
 **************************/
#ifdef BAIKAL_DBM
    // ESPI
    Device(ESPI) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x202A0000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x202A0000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 135 }
        GpioIo(Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 28 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "be,espi" },
          Package() { "cs-gpio", Package() { ^ESPI, Zero, Zero, One } } // as in dts todo: get real gpio
        }
      })

      Device(PR00) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x202A0001)
        Name(_ADR, Zero)
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

#ifdef BAIKAL_DBM
    // AVLSP HDA
    Device(HDA0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x202C0000)
      Name(_CCA, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x202C0000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 86 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "be,cw-hda" }
        }
      })
    }
#endif

#if 0
#if defined (BAIKAL_DBM) || defined (BAIKAL_MBM20) // remove MBM20 before release
    // VDU_LVDS
    Device(VDU0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x202D0000)
      Name(_CCA, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x202D0000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 144, 145 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,vdu" },
          Package() { "lvds-out", One }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "port@0", "PRT0" }
        }
      })
      Name(PRT0, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero },
          Package() { "port", Zero }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "endpoint@0", "EP00" },
          Package() { "endpoint@1", "EP01" }
        }
      })
      Name(EP00, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero },
          Package() { "endpoint", Zero },
          Package() { "remote-endpoint", Package() { ^PNL0, 0, 0 } }
        }
      })
      Name(EP01, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", One },
          Package() { "endpoint", One },
          Package() { "remote-endpoint", Package() { ^PNL0, 0, 1 } }
        }
      })
    }
#endif
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

/**********************************************
 * Missing baikal driver (and maybe need fix) *
 **********************************************/
#if 0
    // VDEC
    Device(VDEC) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x24200000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x24200000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 529 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,d5500-vxd" }
        }
      })
    }
#endif

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
          Package() { "compatible", "baikal,pvt" },
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
          Package() { "compatible", "baikal,pvt" },
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
          Package() { "compatible", "baikal,pvt" },
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

#ifdef BAIKAL_DBM
#if 0
    // XGMAC0
    Device(XGM0) {
      Name(_HID, "AMDI8001")
      Name(_UID, Zero)
      Name(_CCA, One)
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
          Package() { "mac-address", Package() { 0x00, 0x20, 0x13, 0xBA, 0x1C, 0xA1 } },
          Package() { "local-mac-address", Package() { 0x00, 0x20, 0x13, 0xBA, 0x1C, 0xA1 } },
          Package() { "phy-mode", "xgmii" },
          Package() { "be,pcs-mode", "KX4" },
          Package() { "ext-phy-handle", ^MDIO.PR0C },
          Package() { "amd,per-channel-interrupt", One },
          Package() { "amd,speed-set", Zero }
        }
      })
    }

    // XGMAC1
    Device(XGM1) {
      Name(_HID, "AMDI8001")
      Name(_UID, One)
      Name(_CCA, One)
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
          Package() { "mac-address", Package() { 0x00, 0x20, 0x13, 0xBA, 0x1C, 0xA2 } },
          Package() { "local-mac-address", Package() { 0x00, 0x20, 0x13, 0xBA, 0x1C, 0xA2 } },
          Package() { "phy-mode", "xgmii" },
          Package() { "be,pcs-mode", "KX4" },
          Package() { "ext-phy-handle", ^MDIO.PR0E },
          Package() { "amd,per-channel-interrupt", One },
          Package() { "amd,speed-set", Zero }
        }
      })
    }
#endif
#endif

#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
    // GMAC0
    Device(GMC0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x30240000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30240000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 323 }
#ifdef BAIKAL_MBM20
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
#ifdef BAIKAL_MBM20
          Package() { "snps,reset-gpios", Package() {
            ^GMC0, Zero, Zero, One,
          }},
#endif
          Package() { "snps,reset-delays-us", Package() { Zero, 10200, 1000 } }
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
#ifdef BAIKAL_MBM20
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
#ifdef BAIKAL_MBM20
          Package() { "snps,reset-gpios", Package() {
            ^GMC1, Zero, Zero, One,
          }},
#endif
          Package() { "snps,reset-delays-us", Package() { Zero, 10200, 1000 } }
        }
      })

      Device(GPHY) {
        Name(_ADR, Zero)
      }
    }
#endif

#if 0
    // VDU_HDMI
    Device(VDU1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x30260000)
      Name(_CCA, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30260000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 361, 362 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,vdu" }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "port@0", "PRT0" }
        }
      })
      Name(PRT0, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero },
          Package() { "port", Zero }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "endpoint@0", "EP00" }
        }
      })
      Name(EP00, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero },
          Package() { "endpoint", Zero },
          Package() { "remote-endpoint", Package() { ^HDMI, 0, 0 } }
        }
      })
    }

    // HDMI
    Device(HDMI) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x30280000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30280000, 0x20000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 363 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,hdmi" },
          Package() { "reg-io-width", 4 }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "port@0", "PRT0" }
        }
      })
      Name(PRT0, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero },
          Package() { "port", Zero }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "endpoint@0", "EP00" }
        }
      })
      Name(EP00, Package() {
       ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero },
          Package() { "endpoint", Zero },
          Package() { "remote-endpoint", Package() { ^VDU1, 0, 0 } }
        }
      })
    }
#endif

/************************
 * !!!Missing Driver!!! *
 ************************/
#if 0
    // HDMI-OUT
    Device(HOUT) {
      Name(_HID, "PRP0001")
      Name(_UID, 100)
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "hdmi-connector" },
          Package() { "label", "HDMI0 OUT" },
          Package() { "type", "a" }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "port@0", "PRT0" }
        }
      })
      Name(PRT0, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "endpoint@0", "EP00" }
        }
      })
      Name(EP00, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero },
          Package() { "remote-endpoint", Package() { "\\_SB.HDMI", "port@1", "endpoint@0" } }
        }
      })
    }
#endif

#ifdef BAIKAL_DBM
/**************************
 * Baikal driver need fix *
 **************************/
#if 0
    // Internal MDIO
    Device(MDIO) {
      Name(_HID, "PRP0001")
      Name(_UID, 200)
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "be,mdio-gpio" },
          Package() { "mdc-pin", Package() {
            ^GPIO.GPIP, 30, Zero
          }},
          Package() { "mdio-pin", Package() {
            ^GPIO.GPIP, 29, Zero
          }}
        }
      })

      Device(PR0C) {
        Name(_HID, "PRP0001")
        Name(_UID, 201)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "ethernet-phy-ieee802.3-c45" },
            Package() { "reg", 0x0C },
            Package() { "phy-mode", "xgmii" },
            Package() { "mv,line-mode", "KR" },
            Package() { "mv,host-mode", "KX4" }
          }
        })
      }

      Device(PR0E) {
        Name(_HID, "PRP0001")
        Name(_UID, 202)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "ethernet-phy-ieee802.3-c45" },
            Package() { "reg", 0x0E },
            Package() { "phy-mode", "xgmii" },
            Package() { "mv,line-mode", "KR" },
            Package() { "mv,host-mode", "KX4" }
          }
        })
      }
    }
#endif

/**************************
 * Baikal driver need fix *
 **************************/
#if 0
    // Sound
    Device(SND0) {
      Name(_HID, "PRP0001")
      Name(_UID, 300)
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,snd_soc_be" },
          Package() { "baikal,cpu-dai", ^I2S0 },
          Package() { "baikal,audio-codec", ^I2C0.PR18 }
        }
      })
    }
#endif
#endif

#if 0
#if defined (BAIKAL_DBM) || defined (BAIKAL_MBM20) // remove MBM20 before release
    // Panel
    Device(PNL0) {
      Name(_HID, "PRP0001")
      Name(_UID, 400)
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "baikal,panel-lvds" },
          Package() { "width-mm", 223 },
          Package() { "height-mm", 125 },
          Package() { "data-mapping", "vesa-24" },
          /* 1920x1080 @ 60 Hz */
          Package() { "clock-frequency", 148500000 },
          Package() { "hactive", 1920 },
          Package() { "vactive", 1080 },
          Package() { "hsync-len", 44 },
          Package() { "hfront-porch", 88 },
          Package() { "hback-porch", 148 },
          Package() { "vsync-len", 5 },
          Package() { "vfront-porch", 4 },
          Package() { "vback-porch", 36 }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "port@0", "PRT0" }
        }
      })
      Name(PRT0, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero },
          Package() { "port", Zero }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "endpoint@0", "EP00" },
          Package() { "endpoint@1", "EP01" }
        }
      })
      Name(EP00, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", Zero },
          Package() { "endpoint", Zero },
          Package() { "remote-endpoint", Package() { ^VDU0, 0, 0 } }
        }
      })
      Name(EP01, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "reg", One },
          Package() { "endpoint", One },
          Package() { "remote-endpoint", Package() { ^VDU0, 0, 1 } }
        }
      })
    }
#endif
#endif

#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
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
