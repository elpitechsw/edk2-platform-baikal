/** @file
  Copyright (c) 2020 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiPlatform.h"

DefinitionBlock ("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1)
{
  Include ("DsdtInclude.asl")

  /* Timer1 */
  Method (\_SB.TMR1._STA) { Return (0xF) }

  /* Timer2 */
  Method (\_SB.TMR2._STA) { Return (0xF) }

  /* Timer3 */
  Method (\_SB.TMR3._STA) { Return (0xF) }

  /* Timer4 */
  Method (\_SB.TMR4._STA) { Return (0xF) }

  /* PVT2 */
  Method (\_SB.PVT2._STA) { Return (0xF) }

  /* PVT1 */
  Method (\_SB.PVT1._STA) { Return (0xF) }

  /* DDR0 */
  Method (\_SB.DDR0._STA) { Return (Zero) }

  /* DDR1 */
  Method (\_SB.DDR1._STA) { Return (Zero) }

  /* GPIO */
  Method (\_SB.GPIO._STA) { Return (0xF) }
  Method (\_SB.GPIO.GPIP._STA) { Return (0xF) }
  Name (\_SB.GPIO.GPIP._DSD, Package ()
  {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
    Package ()
    {
      Package () { "reg", Zero },
      Package () { "ngpios", 32 },
      Package () { "line-name", "PCIE2_PRSNT#" },
      Package () { "gpio-hog", One },
      Package () { "gpios", Package () { One, One } },
      Package () { "output-high", One },
      Package () { "gpio-line-names", Package () {
        "",             /* GPIO32_0 */
        "PCIE2_PRSNT#", /* GPIO32_1 */
        "",             /* GPIO32_2 */
        "PCIE2_PERST#", /* GPIO32_3 */
        "",             /* GPIO32_4 */
        "",             /* GPIO32_5 */
        "PCIE0_PERST#", /* GPIO32_6 */
        "",             /* GPIO32_7 */
        "LED0",         /* GPIO32_8 */
        "",             /* GPIO32_9 */
        "",             /* GPIO32_10 */
        "",             /* GPIO32_11 */
        "",             /* GPIO32_12 */
        "",             /* GPIO32_13 */
        "",             /* GPIO32_14 */
        "",             /* GPIO32_15 */
#ifdef USE_LVDS
        "LVDS_EN#",     /* GPIO32_16 */
        "BRIGHT_UP",    /* GPIO32_17 */
        "BRIGHT_DOWN",  /* GPIO32_18 */
#else
        "",             /* GPIO32_16 */
        "LVDS_CLK_EN#", /* GPIO32_17 */
        "HDMI_CLK_EN#", /* GPIO32_18 */
#endif
        "ETH0_RST#",    /* GPIO32_19 */
        "ETH1_RST#",    /* GPIO32_20 */
        "",             /* GPIO32_21 */
        "",             /* GPIO32_22 */
        "",             /* GPIO32_23 */
        "",             /* GPIO32_24 */
        "",             /* GPIO32_25 */
        "MIC_DET#",     /* GPIO32_26 */
        "",             /* GPIO32_27 */
        "",             /* GPIO32_28 */
        "HP_DET",       /* GPIO32_29 */
        "",             /* GPIO32_30 */
#ifdef USE_LVDS
        "BRIGHT_TOGGLE" /* GPIO32_31 */
#else
        ""              /* GPIO32_31 */
#endif
      }}
    }
  })

  /* SPI */
  Method (\_SB.SPI0._STA) { Return (0xF) }

  /* DMAC LSP */
  Method (\_SB.DMA0._STA) { Return (0xF) }

  /* DMAC M2M */
  Method (\_SB.DMA1._STA) { Return (0xF) }

  /* UART1 */
  Method (\_SB.COM0._STA) { Return (0xF) }

  /* UART2 */
  Method (\_SB.COM1._STA) { Return (0xF) }

  /* I2C1 */
  Method (\_SB.I2C1._STA) { Return (0xF) }

  /* I2C2 */
  Method (\_SB.I2C2._STA) { Return (0xF) }

  /* SMBUS1 */
  Method (\_SB.SMB0._STA) { Return (0xF) }

  /* SMBUS2 */
  Method (\_SB.SMB1._STA) { Return (0xF) }

  /* ESPI */
  Method (\_SB.ESPI._STA) { Return (Zero) }

  /* I2S */
  Method (\_SB.I2S0._STA) { Return (0xF) }

  /* XGMAC0 */
  Method (\_SB.XGM0._STA) { Return (Zero) }

  /* XGMAC1 */
  Method (\_SB.XGM1._STA) { Return (Zero) }

  /* PVT3 */
  Method (\_SB.PVT3._STA) { Return (0xF) }

  /* PVT0 */
  Method (\_SB.PVT0._STA) { Return (0xF) }

  /* PVTM */
  Method (\_SB.PVTM._STA) { Return (0xF) }

  /* SD/eMMC */
  Method (\_SB.MMC0._STA) { Return (0xF) }

  /* USB2 */
  Method (\_SB.USB2._STA) { Return (0xF) }
  Method (\_SB.USB2.RHUB._STA) { Return (0xF) }
  Method (\_SB.USB2.RHUB.PRT1._STA) { Return (0xF) }
  Method (\_SB.USB2.RHUB.PRT2._STA) { Return (0xF) }

  /* USB3 */
  Method (\_SB.USB3._STA) { Return (0xF) }
  Method (\_SB.USB3.RHUB._STA) { Return (0xF) }
  Method (\_SB.USB3.RHUB.PRT1._STA) { Return (0xF) }
  Method (\_SB.USB3.RHUB.PRT2._STA) { Return (0xF) }
  Method (\_SB.USB3.RHUB.PRT3._STA) { Return (0xF) }
  Method (\_SB.USB3.RHUB.PRT4._STA) { Return (0xF) }

  /* USB PHY */
  Method (\_SB.UPHY._STA) { Return (0xF) }
  Method (\_SB.UPHY.PHY0._STA) { Return (0xF) }
  Method (\_SB.UPHY.PHY1._STA) { Return (0xF) }
  Method (\_SB.UPHY.PHY2._STA) { Return (0xF) }

  /* SATA0 */
  Method (\_SB.SAT0._STA) { Return (0xF) }

  /* SATA1 */
  Method (\_SB.SAT1._STA) { Return (0xF) }

  /* GMAC0 */
  Method (\_SB.GMC0._STA) { Return (0xF) }
  Method (\_SB.GMC0.GPHY._STA) { Return (0xF) }
  Name (\_SB.GMC0.GPIO, ResourceTemplate ()
  {
    GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 19 }
  })
  Name (\_SB.GMC0.RSGP, Package ()
  {
    \_SB.GMC0, Zero, Zero, One
  })
  Name (\_SB.GMC0.RSDL, Package ()
  {
    Zero, 10000, 50000
  })

  /* GMAC1 */
  Method (\_SB.GMC1._STA) { Return (0xF) }
  Method (\_SB.GMC1.GPHY._STA) { Return (0xF) }
  Name (\_SB.GMC1.GPIO, ResourceTemplate ()
  {
    GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 20 }
  })
  Name (\_SB.GMC1.RSGP, Package ()
  {
    \_SB.GMC1, Zero, Zero, One
  })
  Name (\_SB.GMC1.RSDL, Package ()
  {
    Zero, 10000, 50000
  })

  /* GPU MALI */
  Method (\_SB.GPU0._STA) { Return (0xF) }

  /* VDEC */
  Method (\_SB.VDEC._STA) { Return (0xF) }

  /* HDA */
  Method (\_SB.HDA0._STA) { Return (Zero) }

  /* HDMI */
  Method (\_SB.HDMI._STA) { Return (0xF) }

  /* VDU */
  Method (\_SB.VDU0._STA) { Return (0xF) }
  Name (\_SB.VDU0.GPIO, ResourceTemplate ()
  {
    GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 16 }
  })
  Name (\_SB.VDU0.ENGP, Package ()
  {
    \_SB.VDU0, Zero, Zero, One
  })
  Name (\_SB.VDU0.LVLN, 2)

  /* Additional devices */
  Device (\_SB.I2C1.PR08)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20250008)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      I2CSerialBusV2 (0x08, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1")
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "tp,mitx2-bmc" }
      }
    })
  }

  Device (\_SB.I2C1.PR1A)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x2025001A)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      I2CSerialBusV2 (0x1A, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1")
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "nuvoton,nau8822" }
      }
    })
  }

  Device (\_SB.I2C1.PR50)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20250050)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      I2CSerialBusV2 (0x50, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1")
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "nxp,pca9670" }
      }
    })
  }

  Device (\_SB.I2C1.PR51)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20250051)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      I2CSerialBusV2 (0x51, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1")
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", Package () { "nxp,pcf2129", "nxp,pcf2127" } }
      }
    })
  }

  Device (\_SB.I2C1.PR53)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20250053)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      I2CSerialBusV2 (0x53, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1")
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "atmel,24c32" },
        Package () { "pagesize", 32 }
      }
    })
  }

  /* SND BAIKAL SIMPLE */
  Device (\_SB.SND0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 900)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 26, 29 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,simple-audio-card" },
        Package () { "mic-det-gpio", Package () { \_SB.SND0, Zero, Zero, One } },
        Package () { "hp-det-gpio", Package () { \_SB.SND0, Zero, One, Zero } },
        Package () { "baikal,cpu-dai", \_SB.I2S0 },
        Package () { "baikal,audio-codec", \_SB.I2C1.PR1A },
        Package () { "baikal,codec-name", "nau8822-hifi" }
      }
    })
  }

  Device (\_SB.LEDS)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 600)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "gpio-leds" }
      }
    })

    Device (LED0)
    {
      Name (_ADR, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 8 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "default-state", "keep" },
          Package () { "label", "led0" },
          Package () { "gpios", Package () { \_SB.LEDS.LED0, Zero, Zero, Zero } }
        }
      })
    }
  }

  Device (\_SB.VDU0.PNL0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 700)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,panel-lvds" },
        Package () { "width-mm", 223 },
        Package () { "height-mm", 125 },
        Package () { "data-mapping", "vesa-24" },
        Package () { "clock-frequency", 148500000 },
        Package () { "hactive", 1920 },
        Package () { "vactive", 1080 },
        Package () { "hsync-len", 44 },
        Package () { "hfront-porch", 88 },
        Package () { "hback-porch", 148 },
        Package () { "vsync-len", 5 },
        Package () { "vfront-porch", 4 },
        Package () { "vback-porch", 36 }
      }
    })
  }

  Device (\_SB.VDU0.BCKL)
  {
    Name (_ADR, 0)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "min-brightness-level", 10 },
        Package () { "default-brightness-level", 60 },
        Package () { "brightness-level-step", 2 },
        Package () { "pwm-frequency", 20000 }
      }
    })
  }

  Device (\_SB.VDU0.KEYS)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 701)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 18, 17, 31 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "gpio-keys" },
        Package () { "autorepeat", One }
      }
    })

    Device (BTN0)
    {
      Name (_ADR, 0)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "label", "Brightness Down Button" },
          Package () { "linux,code", 224 },
          Package () { "debounce-interval", 50 },
          Package () { "gpios", Package () { \_SB.VDU0.KEYS, Zero, Zero, One } }
        }
      })
    }

    Device (BTN1)
    {
      Name (_ADR, 1)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "label", "Brightness Up Button" },
          Package () { "linux,code", 225 },
          Package () { "debounce-interval", 50 },
          Package () { "gpios", Package () { \_SB.VDU0.KEYS, Zero, One, One } }
        }
      })
    }

    Device (BTN2)
    {
      Name (_ADR, 2)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "label", "Brightness Toggle Button" },
          Package () { "linux,code", 0x1AF },
          Package () { "debounce-interval", 50 },
          Package () { "gpios", Package () { \_SB.VDU0.KEYS, Zero, 2, One } }
        }
      })
    }
  }
}
