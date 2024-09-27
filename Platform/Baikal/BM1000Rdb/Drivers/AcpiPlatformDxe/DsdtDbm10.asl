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
      Package () { "gpio-line-names", Package () {
        "",             /* GPIO32_0 */
        "",             /* GPIO32_1 */
        "",             /* GPIO32_2 */
        "",             /* GPIO32_3 */
        "",             /* GPIO32_4 */
        "",             /* GPIO32_5 */
        "",             /* GPIO32_6 */
        "",             /* GPIO32_7 */
        "",             /* GPIO32_8 */
        "",             /* GPIO32_9 */
        "",             /* GPIO32_10 */
        "",             /* GPIO32_11 */
        "",             /* GPIO32_12 */
        "",             /* GPIO32_13 */
        "",             /* GPIO32_14 */
        "",             /* GPIO32_15 */
        "",             /* GPIO32_16 */
        "",             /* GPIO32_17 */
        "",             /* GPIO32_18 */
        "",             /* GPIO32_19 */
        "",             /* GPIO32_20 */
        "",             /* GPIO32_21 */
        "",             /* GPIO32_22 */
        "",             /* GPIO32_23 */
        "FLASH0_CS#",   /* GPIO32_24 */
        "",             /* GPIO32_25 */
        "",             /* GPIO32_26 */
        "",             /* GPIO32_27 */
        "",             /* GPIO32_28 */
        "",             /* GPIO32_29 */
        "",             /* GPIO32_30 */
        ""              /* GPIO32_31 */
      }}
    }
  })

  /* SPI */
  Method (\_SB.SPI0._STA) { Return (0xF) }
  Name (\_SB.SPI0.CSGP, Package ()
  {
    \_SB.SPI0, Zero, Zero, One,
    \_SB.SPI0, Zero, One, One,
    \_SB.SPI0, Zero, 2, One,
    \_SB.SPI0, Zero, 3, One
  })

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
  Method (\_SB.ESPI._STA) { Return (0xF) }
  Name (\_SB.ESPI.CSGP, Package ()
  {
    \_SB.ESPI, Zero, Zero, One
  })

  /* PVT3 */
  Method (\_SB.PVT3._STA) { Return (0xF) }

  /* PVT0 */
  Method (\_SB.PVT0._STA) { Return (0xF) }

  /* PVTM */
  Method (\_SB.PVTM._STA) { Return (0xF) }

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

  /* GMAC1 */
  Method (\_SB.GMC1._STA) { Return (0xF) }
  Method (\_SB.GMC1.GPHY._STA) { Return (0xF) }

  /* GPU MALI */
  Method (\_SB.GPU0._STA) { Return (0xF) }

  /* VDEC */
  Method (\_SB.VDEC._STA) { Return (0xF) }

  /* HDMI */
  Method (\_SB.HDMI._STA) { Return (0xF) }

  /* VDU */
  Method (\_SB.VDU0._STA) { Return (0xF) }

  /* Additional devices */
  Device (\_SB.SPI0.PR00)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20210000)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      SPISerialBusV2 (Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 12500000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.SPI0")
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "baikal,partitions", Package ()
        {
          "BL1",       0x000000, 0x010000,
          "FIP",       0x040000, 0x1C0000,
          "DDR0 cfg",  0x200000, 0x040000,
          "DDR1 cfg",  0x240000, 0x040000,
          "UEFI vars", 0x280000, 0x0C0000
        }}
      }
    })
  }

  Device (\_SB.I2C1.PR56)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20250056)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      I2CSerialBusV2 (0x56, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1")
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "abracon,abeoz9" }
      }
    })
  }

  Device (\_SB.ESPI.PR00)
  {
    Name (_HID, "SPT0001")
    Name (_UID, 0x202A0000)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      SPISerialBusV2 (Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 10000000, ClockPolarityLow, ClockPhaseSecond, "\\_SB.ESPI")
    })
  }

  Include ("MultimediaMezz.asl")
  Include ("XgbeMezz.asl")
}
