#include "AcpiPlatformDxe.h"

DefinitionBlock("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1) {
  Scope(_SB_) {
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

    // PCIe LCRU
    Device(LCRU) {
      Name(_ADR, 0x02000000)
      Name(_UID, 0x02000000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x02000000, 0x80000)
      })
    }

    // PCIe x4 #0
    Device(PCI0) {
      Name(_HID, EISAID("PNP0A08"))
      Name(_CID, EISAID("PNP0A03"))
      Name(_UID, 0x02200000)
      Name(_ADR, 0x02200000)
      Name(_CCA, Zero)
      Name(_SEG, BAIKAL_ACPI_PCIE_X4_0_SEGMENT)
      Name(_BBN, Zero)

      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x40000000, 0x10000000)
        Memory32Fixed(ReadWrite, 0x02200000, 0x00001000)
        QWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite, Zero, 0x40000000, 0x7FFFFFFF, 0x3C0000000, 0x40000000)
        QWordIo(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange, Zero, Zero, 0x000FFFFF, 0x70000000, 0x00100000, ,,, TypeTranslation)
        WordBusNumber(ResourceProducer, MinFixed, MaxFixed, PosDecode, Zero, Zero, 255, Zero, 256)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 458, 461 }
#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
        GpioIo(Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 6 }
#endif
      })

      Name(NUML, 4)
      Name(NUMV, 4)
      Name(LCRU, Package() { ^LCRU, 0 })

      Name(SUPP, Zero) // PCI _OSC Support Field value
      Name(CTRL, Zero) // PCI _OSC Control Field value

      Method(_OSC, 4) {
        // Check for proper UUID
        If(LEqual(Arg0, ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) {
          // Create DWord-adressable fields from the Capabilities Buffer
          CreateDWordField(Arg3, Zero, CDW1)
          CreateDWordField(Arg3, 4, CDW2)
          CreateDWordField(Arg3, 8, CDW3)

          // Save Capabilities DWord2 & 3
          Store(CDW2, SUPP)
          Store(CDW3, CTRL)

          // Only allow native hot plug control if OS supports:
          // * ASPM
          // * Clock PM
          // * MSI/MSI-X
          If(LNotEqual(And(SUPP, 0x16), 0x16)) {
            And(CTRL, 0x1E, CTRL) // Mask bit 0 (and undefined bits)
          }

          // Always allow native PME, AER (no dependencies)

          // Never allow SHPC (no SHPC controller in this system)
          And(CTRL, 0x1D, CTRL)

          // Unknown revision
          If(LNotEqual(Arg1, One)) {
            Or(CDW1, 0x08, CDW1)
          }

          // Capabilities bits were masked
          If(LNotEqual(CDW3, CTRL)) {
            Or(CDW1, 0x10, CDW1)
          }

          // Update DWORD3 in the buffer
          Store(CTRL, CDW3)
          Return(Arg3)
        } Else {
          // Unrecognized UUID
          Or(CDW1, 4, CDW1)
          Return(Arg3)
        }
      }
    }

#ifdef BAIKAL_DBM
    // PCIe x4 #1
    Device(PCI1) {
      Name(_HID, EISAID("PNP0A08"))
      Name(_CID, EISAID("PNP0A03"))
      Name(_UID, 0x02210000)
      Name(_ADR, 0x02210000)
      Name(_CCA, Zero)
      Name(_SEG, BAIKAL_ACPI_PCIE_X4_1_SEGMENT)
      Name(_BBN, Zero)

      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x50000000, 0x10000000)
        Memory32Fixed(ReadWrite, 0x02210000, 0x00001000)
        QWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite, Zero, 0x40000000, 0x7FFFFFFF, 0x4C0000000, 0x40000000)
        QWordIo(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange, Zero, 0x00100000, 0x001FFFFF, 0x70000000, 0x00100000, ,,, TypeTranslation)
        WordBusNumber(ResourceProducer, MinFixed, MaxFixed, PosDecode, Zero, Zero, 255, Zero, 256)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 434, 437 }
      })

      Name(NUML, 4)
      Name(NUMV, 4)
      Name(LCRU, Package() { ^LCRU, One })

      Name(SUPP, Zero) // PCI _OSC Support Field value
      Name(CTRL, Zero) // PCI _OSC Control Field value

      Method(_OSC, 4) {
        // Check for proper UUID
        If(LEqual(Arg0, ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) {
          // Create DWord-adressable fields from the Capabilities Buffer
          CreateDWordField(Arg3, Zero, CDW1)
          CreateDWordField(Arg3, 4, CDW2)
          CreateDWordField(Arg3, 8, CDW3)

          // Save Capabilities DWord2 & 3
          Store(CDW2, SUPP)
          Store(CDW3, CTRL)

          // Only allow native hot plug control if OS supports:
          // * ASPM
          // * Clock PM
          // * MSI/MSI-X
          If(LNotEqual(And(SUPP, 0x16), 0x16)) {
            And(CTRL, 0x1E, CTRL) // Mask bit 0 (and undefined bits)
          }

          // Always allow native PME, AER (no dependencies)

          // Never allow SHPC (no SHPC controller in this system)
          And(CTRL, 0x1D, CTRL)

          // Unknown revision
          If(LNotEqual(Arg1, One)) {
            Or(CDW1, 0x08, CDW1)
          }

          // Capabilities bits were masked
          If(LNotEqual(CDW3, CTRL)) {
            Or(CDW1, 0x10, CDW1)
          }

          // Update DWORD3 in the buffer
          Store(CTRL, CDW3)
          Return(Arg3)
        } Else {
          // Unrecognized UUID
          Or(CDW1, 4, CDW1)
          Return(Arg3)
        }
      }
    }
#endif

    // PCIe x8
    Device(PCI2) {
      Name(_HID, EISAID("PNP0A08"))
      Name(_CID, EISAID("PNP0A03"))
      Name(_UID, 0x02220000)
      Name(_ADR, 0x02220000)
      Name(_CCA, Zero)
      Name(_SEG, BAIKAL_ACPI_PCIE_X8_SEGMENT)
      Name(_BBN, Zero)

      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x60000000, 0x10000000)
        Memory32Fixed(ReadWrite, 0x02220000, 0x00001000)
        QWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite, Zero, 0x80000000, 0xFFFFFFFF, 0x580000000, 0x80000000)
        QWordIo(ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange, Zero, 0x00200000, 0x002FFFFF, 0x70000000, 0x00100000, ,,, TypeTranslation)
        WordBusNumber(ResourceProducer, MinFixed, MaxFixed, PosDecode, Zero, Zero, 255, Zero, 256)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 410, 413 }
#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
        GpioIo(Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 3 }
#endif
      })

      Name(NUML, 8)
      Name(NUMV, 4)
      Name(LCRU, Package() { ^LCRU, 2 })

      Name(SUPP, Zero) // PCI _OSC Support Field value
      Name(CTRL, Zero) // PCI _OSC Control Field value

      Method(_OSC, 4) {
        // Check for proper UUID
        If(LEqual(Arg0, ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) {
          // Create DWord-adressable fields from the Capabilities Buffer
          CreateDWordField(Arg3, Zero, CDW1)
          CreateDWordField(Arg3, 4, CDW2)
          CreateDWordField(Arg3, 8, CDW3)

          // Save Capabilities DWord2 & 3
          Store(CDW2, SUPP)
          Store(CDW3, CTRL)

          // Only allow native hot plug control if OS supports:
          // * ASPM
          // * Clock PM
          // * MSI/MSI-X
          If(LNotEqual(And(SUPP, 0x16), 0x16)) {
            And(CTRL, 0x1E, CTRL) // Mask bit 0 (and undefined bits)
          }

          // Always allow native PME, AER (no dependencies)

          // Never allow SHPC (no SHPC controller in this system)
          And(CTRL, 0x1D, CTRL)

          // Unknown revision
          If(LNotEqual(Arg1, One)) {
            Or(CDW1, 0x08, CDW1)
          }

          // Capabilities bits were masked
          If(LNotEqual(CDW3, CTRL)) {
            Or(CDW1, 0x10, CDW1)
          }

          // Update DWORD3 in the buffer
          Store(CTRL, CDW3)
          Return(Arg3)
        } Else {
          // Unrecognized UUID
          Or(CDW1, 4, CDW1)
          Return(Arg3)
        }
      }
    }

    // PVT2
    Device(PVT2) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x0A200000)
      Name(_ADR, 0x0A200000)
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
      Name(_ADR, 0x0C200000)
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
      Name(_ADR, 0x0E200000)
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
      Name(_UID, 0x20200000)
      Name(_ADR, 0x20200000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20200000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 131 }
      })

      // GPIO port
      Device(GPIP) {
        Name(_UID, 0x20200001)
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
      Name(_UID, 0x20210000)
      Name(_ADR, 0x20210000)
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
        Name(_UID, 0x20210001)
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
      Name(_ADR, 0x20220000)
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
      Name(_UID, 0x20230000)
      Name(_ADR, 0x20230000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20230000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 133 }
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

    // UART2
    Device(COM1) {
      Name(_HID, "APMC0D08")
      Name(_UID, 0x20240000)
      Name(_ADR, 0x20240000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20240000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 134 }
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

    // I2C0
    Device(I2C0) {
      Name(_HID, "APMC0D0F")
      Name(_UID, 0x20250000)
      Name(_ADR, 0x20250000)
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
            Package() { "compatible", Package() { "tp,mitx2-bmc", "t-platforms,mitx2-bmc" } }
          }
        })
      }

      Device(PR1A) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x2025001A)
        Name(_ADR, 0x1A)
        Name(_CRS, ResourceTemplate() {
          I2CSerialBusV2(0x1a, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
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
      Name(_UID, 0x20260000)
      Name(_ADR, 0x20260000)
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
      Name(_ADR, 0x20270000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20270000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 142 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "be,smbus" },
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
      Name(_ADR, 0x20280000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20280000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 143 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "be,smbus" },
          Package() { "clock-frequency", 100000 }
        }
      })
      Name(CLK, 50000000)
    }

/*************************
 * DT-only driver device *
 *************************/
#if 0
    // Timer1
    Device(TMR1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x20290000)
      Name(_ADR, 0x20290000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20290000, 0x14)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 127 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "snps,dw-apb-timer-osc" },
          Package() { "clock-frequency", 50000000 }
        }
      })
    }
#endif

/*************************
 * DT-only driver device *
 *************************/
#if 0
    // Timer2
    Device(TMR2) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x20290014)
      Name(_ADR, 0x20290014)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20290014, 0x14)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 128 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "snps,dw-apb-timer-sp" },
          Package() { "clock-frequency", 50000000 }
        }
      })
    }
#endif

/*************************
 * DT-only driver device *
 *************************/
#if 0
    // Timer3
    Device(TMR3) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x20290028)
      Name(_ADR, 0x20290028)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20290028, 0x14)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 129 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "snps,dw-apb-timer-sp" },
          Package() { "clock-frequency", 50000000 }
        }
      })
    }
#endif

/*************************
 * DT-only driver device *
 *************************/
#if 0
    // Timer4
    Device(TMR4) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x2029003C)
      Name(_ADR, 0x2029003C)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2029003C, 0x14)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 130 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "snps,dw-apb-timer-sp" },
          Package() { "clock-frequency", 50000000 }
        }
      })
    }
#endif

/**************************
 * Baikal driver need fix *
 **************************/
#ifdef BAIKAL_DBM
    // ESPI
    Device(ESPI) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x202A0000)
      Name(_ADR, 0x202A0000)
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

/*******************
 * Not used device *
 *******************/
#if 0
    // AVLSP DMAC
    Device(DMAC) {
      Name(_HID, "80862286")
      Name(_UID, 0x202B0000)
      Name(_ADR, 0x202B0000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x202B0000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 41 }
      })
    }
#endif

#ifdef BAIKAL_DBM
#if 0
    // AVLSP HDA
    Device(HDA0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x202C0000)
      Name(_ADR, 0x202C0000)
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
#endif

#if defined(BAIKAL_DBM) || defined(BAIKAL_MBM20) // remove MBM20 before release
    // VDU_LVDS
    Device(VDU0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x202D0000)
      Name(_ADR, 0x202D0000)
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

    // SD/eMMC
    Device(MMC0) {
      Name(_HID, "PRP0001")
      Name(_CID, "PNP0D40")
      Name(_UID, 0x202E0000)
      Name(_ADR, 0x202E0000)
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
    // DDR2
    Device(DDR1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x22200000)
      Name(_ADR, 0x22200000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x22200000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 171, 172, 173, 174, 175, 176 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", Package() { "be,emc", "be,memory-controller" } }
        }
      })
    }
#endif

/**********************************************
 * Missing baikal driver (and maybe need fix) *
 **********************************************/
#if 0
    // VDEC
    Device(VDEC) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x24200000)
      Name(_ADR, 0x24200000)
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
      Name(_ADR, 0x26200000)
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
      Name(_ADR, 0x28200000)
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
      Name(_ADR, 0x2A060000)
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
      Name(_CID, "PNP0D20")
      Name(_UID, 0x2C400000)
      Name(_ADR, 0x2C400000)
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
      Name(_UID, 0x2C500000)
      Name(_ADR, 0x2C500000)
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
      Name(_ADR, 0x2C600000)
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
      Name(_ADR, 0x2C610000)
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

/*******************
 * Not used device *
 *******************/
#if 0
    // DMA-330
    Device(DMA0) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x2C620000)
      Name(_ADR, 0x2C620000)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x2C620000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 255, 256, 257, 258, 259, 260, 261, 262, 263}
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", Package() { "arm,pl330", "arm,primecell" } },
          Package() { "#dma-cells", One },
          Package() { "#dma-channels", 8 },
          Package() { "#dma-requests", 32 }
        }
      })
    }
#endif

#ifdef BAIKAL_DBM
#if 0
    // XGMAC0
    Device(XGM0) {
      Name(_HID, "AMDI8001")
      Name(_UID, 0x30200000)
      Name(_ADR, 0x30200000)
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
      Name(_UID, 0x30220000)
      Name(_ADR, 0x30220000)
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
      Name(_ADR, 0x30240000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30240000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 323 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", Package() { "be,dwmac", "snps,dwmac-3.710", "snps,dwmac" } },
          Package() { "max-speed", 1000 },
          Package() { "snps,fixed-burst", One },
          Package() { "mac-address", Package() { Zero, Zero, Zero, Zero, Zero, Zero } },
          Package() { "local-mac-address", Package() { Zero, Zero, Zero, Zero, Zero, Zero } },
          Package() { "phy-mode", "rgmii-id" },
          Package() { "phy-handle", MDIO.GPHY },
#ifdef BAIKAL_MBM10
          Package() { "snps,reset-gp-out", One },
          Package() { "snps,reset-active-low", One },
#elif defined (BAIKAL_MBM20)
          Package() { "snps,reset-gpios", Package() { ^GPIO.GPIP, 19, One } },
#endif
          Package() { "snps,reset-delays-us", Package() { Zero, 10200, 1000 } }
        }
      })

      Device(MDIO) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x30240001)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "snps,dwmac-mdio" }
          }
        })

        Device(GPHY) {
          Name(_HID, "PRP0001")
          Name(_UID, 0x30240002)
          Name(_DSD, Package() {
            ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package() {
              Package() { "compatible", Package() { "micrel,ksz9031", "ethernet-phy-id0022.1620", "ethernet-phy-ieee802.3-c22" } },
              Package() { "reg", 3 },
              Package() { "txd0-skew-ps", Zero },
              Package() { "txd1-skew-ps", Zero },
              Package() { "txd2-skew-ps", Zero },
              Package() { "txd3-skew-ps", Zero },
              Package() { "txc-skew-ps", 0xff }
            }
          })
        }
      }
    }

    // GMAC1
    Device(GMC1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x30250000)
      Name(_ADR, 0x30250000)
      Name(_CCA, One)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x30250000, 0x10000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 324 }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", Package() { "be,dwmac", "snps,dwmac-3.710", "snps,dwmac" } },
          Package() { "max-speed", 1000 },
          Package() { "snps,fixed-burst", One },
          Package() { "mac-address", Package() { Zero, Zero, Zero, Zero, Zero, Zero } },
          Package() { "local-mac-address", Package() { Zero, Zero, Zero, Zero, Zero, Zero } },
          Package() { "phy-mode", "rgmii-id" },
          Package() { "phy-handle", MDIO.GPHY },
#ifdef BAIKAL_MBM10
          Package() { "snps,reset-gp-out", One },
          Package() { "snps,reset-active-low", One },
#elif defined (BAIKAL_MBM20)
          Package() { "snps,reset-gpios", Package() { ^GPIO.GPIP, 20, One } },
#endif
          Package() { "snps,reset-delays-us", Package() { Zero, 10200, 1000 } }
        }
      })

      Device(MDIO) {
        Name(_HID, "PRP0001")
        Name(_UID, 0x30250001)
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package() { "compatible", "snps,dwmac-mdio" }
          }
        })

        Device(GPHY) {
          Name(_HID, "PRP0001")
          Name(_UID, 0x30250002)
          Name(_DSD, Package() {
            ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
            Package() {
              Package() { "compatible", Package() { "micrel,ksz9031", "ethernet-phy-id0022.1620", "ethernet-phy-ieee802.3-c22" } },
              Package() { "reg", 3 },
              Package() { "txd0-skew-ps", Zero },
              Package() { "txd1-skew-ps", Zero },
              Package() { "txd2-skew-ps", Zero },
              Package() { "txd3-skew-ps", Zero },
              Package() { "txc-skew-ps", 0xff }
            }
          })
        }
      }
    }
#endif

    // VDU_HDMI
    Device(VDU1) {
      Name(_HID, "PRP0001")
      Name(_UID, 0x30260000)
      Name(_ADR, 0x30260000)
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
      Name(_ADR, 0x30280000)
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
            Package() { "compatible", Package() { "marvell,88x2222", "ethernet-phy-ieee802.3-c45" } },
            Package() { "reg", 0x0c },
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
            Package() { "compatible", Package() { "marvell,88x2222", "ethernet-phy-ieee802.3-c45" } },
            Package() { "reg", 0x0e },
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

#if defined (BAIKAL_DBM) || defined (BAIKAL_MBM20)
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

#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
/*************************
 * DT-only driver device *
 *************************/
#if 0
    // Generic sound device
    Device(SND0) {
      Name(_HID, "PRP0001")
      Name(_UID, 500)
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "compatible", "simple-audio-card" },
          Package() { "simple-audio-card,name", "MITX-Sound-Card" },
          Package() { "simple-audio-card,widgets", Package() {
            "Microphone", "Mic Jack", "Headphone", "Headphones",
            "Speaker", "AUX Out", "Line", "Line In"
          }},
          Package() { "simple-audio-card,routing", Package() {
            "Headphones", "RHP", "Headphones", "LHP",
            "AUX Out", "AUXOUT1", "AUX Out", "AUXOUT2",
            "L2", "Mic Jack", "R2", "Mic Jack",
            "LAUX", "Line In", "RAUX", "Line In"
          }},
          Package() { "simple-audio-card,hp-det-gpio", Package() {
#ifdef BAIKAL_MBM10
            ^GPIO.GPIP, 27, Zero
#elif defined (BAIKAL_MBM20)
            ^GPIO.GPIP, 29, Zero
#endif
          }},
          Package() { "simple-audio-card,mic-det-gpio", Package() {
            ^GPIO.GPIP, 26, One
          }},
          Package() { "simple-audio-card,format", "i2s" }
        },
        ToUUID("dbb8e3e6-5886-4ba6-8795-1319f52a966b"),
        Package() {
          Package() { "simple-audio-card,bitclock-master", "CDEC" },
          Package() { "simple-audio-card,frame-master", "CDEC" }
        }
      })
      Name(CDEC, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "sound-dai", Package() { ^I2C0.PR1A, 0 } }
        }
      })
      Name(SCPU, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package() { "sound-dai", ^I2S0 }
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
