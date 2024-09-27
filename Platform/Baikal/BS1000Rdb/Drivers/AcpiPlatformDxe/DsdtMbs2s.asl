/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <BS1000.h>
#include "AcpiPlatform.h"
#include "Dsdt.h"

/* Chip0 */
#define BAIKAL_SPI_OFFSET_S0  0

#define P00C_INTERRUPT_ENABLE  1
#define P00D_INTERRUPT_ENABLE  1
#define P00E_INTERRUPT_ENABLE  1
#define P00F_INTERRUPT_ENABLE  1
#define P010_INTERRUPT_ENABLE  0
#define P011_INTERRUPT_ENABLE  0
#define P012_INTERRUPT_ENABLE  0
#define P013_INTERRUPT_ENABLE  0
#define P014_INTERRUPT_ENABLE  0
#define P015_INTERRUPT_ENABLE  0
#define P016_INTERRUPT_ENABLE  0

#define PC00_INTERRUPT_ENABLE  1
#define PC01_INTERRUPT_ENABLE  1
#define PC02_INTERRUPT_ENABLE  1
#define PC03_INTERRUPT_ENABLE  1
#define PC04_INTERRUPT_ENABLE  1
#define PC05_INTERRUPT_ENABLE  1
#define PC06_INTERRUPT_ENABLE  1
#define PC07_INTERRUPT_ENABLE  1
#define PC08_INTERRUPT_ENABLE  1
#define PC09_INTERRUPT_ENABLE  1
#define PC0A_INTERRUPT_ENABLE  1
#define PC0B_INTERRUPT_ENABLE  1
#define PC0C_INTERRUPT_ENABLE  1
#define PC0D_INTERRUPT_ENABLE  1

/* Chip1 */
#define BAIKAL_SPI_OFFSET_S1  320

#define P10C_INTERRUPT_ENABLE  1
#define P10D_INTERRUPT_ENABLE  1
#define P10E_INTERRUPT_ENABLE  1
#define P10F_INTERRUPT_ENABLE  0
#define P110_INTERRUPT_ENABLE  0
#define P111_INTERRUPT_ENABLE  0
#define P112_INTERRUPT_ENABLE  0
#define P113_INTERRUPT_ENABLE  0
#define P114_INTERRUPT_ENABLE  0
#define P115_INTERRUPT_ENABLE  0
#define P116_INTERRUPT_ENABLE  0

#define PC10_INTERRUPT_ENABLE  1
#define PC11_INTERRUPT_ENABLE  1
#define PC12_INTERRUPT_ENABLE  1
#define PC13_INTERRUPT_ENABLE  1
#define PC14_INTERRUPT_ENABLE  1
#define PC15_INTERRUPT_ENABLE  1
#define PC16_INTERRUPT_ENABLE  1
#define PC17_INTERRUPT_ENABLE  1
#define PC18_INTERRUPT_ENABLE  1
#define PC19_INTERRUPT_ENABLE  1
#define PC1A_INTERRUPT_ENABLE  0
#define PC1B_INTERRUPT_ENABLE  1
#define PC1C_INTERRUPT_ENABLE  0
#define PC1D_INTERRUPT_ENABLE  1

DefinitionBlock ("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1)
{
  /* Chip0 */
  BAIKAL_DSDT_CHIP_NODES(0)
  Scope (\_SB)
  {
    /* Cluster PVT */
    Method (\_SB.P000._STA) { Return (0xF) }
    Method (\_SB.P001._STA) { Return (0xF) }
    Method (\_SB.P002._STA) { Return (0xF) }
    Method (\_SB.P003._STA) { Return (0xF) }
    Method (\_SB.P004._STA) { Return (0xF) }
    Method (\_SB.P005._STA) { Return (0xF) }
    Method (\_SB.P006._STA) { Return (0xF) }
    Method (\_SB.P007._STA) { Return (0xF) }
    Method (\_SB.P008._STA) { Return (0xF) }
    Method (\_SB.P009._STA) { Return (0xF) }
    Method (\_SB.P00A._STA) { Return (0xF) }
    Method (\_SB.P00B._STA) { Return (0xF) }

    /* PCIe PVT */
    Method (\_SB.P00C._STA) { Return (0xF) }
    Method (\_SB.P00D._STA) { Return (0xF) }
    Method (\_SB.P00E._STA) { Return (0xF) }
    Method (\_SB.P00F._STA) { Return (0xF) }
    Method (\_SB.P010._STA) { Return (0xF) }

    /* DDR PVT */
    Method (\_SB.P011._STA) { Return (0xF) }
    Method (\_SB.P012._STA) { Return (0xF) }
    Method (\_SB.P013._STA) { Return (0xF) }
    Method (\_SB.P014._STA) { Return (0xF) }
    Method (\_SB.P015._STA) { Return (0xF) }
    Method (\_SB.P016._STA) { Return (0xF) }

    /* Timer1 */
    Method (\_SB.TM01._STA) { Return (0xF) }

    /* Timer2 */
    Method (\_SB.TM02._STA) { Return (0xF) }

    /* Timer3 */
    Method (\_SB.TM03._STA) { Return (0xF) }

    /* Timer4 */
    Method (\_SB.TM04._STA) { Return (0xF) }

    /* WDT */
    Method (\_SB.WDT0._STA) { Return (0xF) }

    /* DDR0 */
    Method (\_SB.DR00._STA) { Return (Zero) }

    /* DDR1 */
    Method (\_SB.DR01._STA) { Return (Zero) }

    /* DDR2 */
    Method (\_SB.DR02._STA) { Return (Zero) }

    /* DDR3 */
    Method (\_SB.DR03._STA) { Return (Zero) }

    /* DDR4 */
    Method (\_SB.DR04._STA) { Return (Zero) }

    /* DDR5 */
    Method (\_SB.DR05._STA) { Return (Zero) }

    /* USB OHCI */
    Method (\_SB.UB00._STA) { Return (0xF) }

    /* USB EHCI */
    Method (\_SB.UB01._STA) { Return (0xF) }

    /* QSPI_1 */
    Method (\_SB.SP00._STA) { Return (0xF) }

    /* GPIO32 */
    Method (\_SB.GP00._STA) { Return (0xF) }
    Method (\_SB.GP00.GPIP._STA) { Return (0xF) }

    /* SMBUS_I2C2 */
    Method (\_SB.I202._STA) { Return (0xF) }

    /* SMBUS_I2C3 */
    Method (\_SB.I203._STA) { Return (0xF) }

    /* SMBUS_I2C4 */
    Method (\_SB.I204._STA) { Return (0xF) }

    /* UART_A1 */
    Method (\_SB.SR00._STA) { Return (0xF) }

    /* UART_A2 */
    Method (\_SB.SR01._STA) { Return (Zero) }

    /* GMAC0 */
    Method (\_SB.GM00._STA) { Return (0xF) }
    Method (\_SB.GM00.GPHY._STA) { Return (0xF) }

    /* GMAC1 */
    Method (\_SB.GM01._STA) { Return (0xF) }
    Method (\_SB.GM01.GPHY._STA) { Return (0xF) }

    /* MUX */
    Method (\_SB.MXC0._STA) { Return (Zero) }

    /* MUX0 */
    Scope (\_SB) {
      Method (\_SB.MX00._STA) { Return (Zero) }

      /* GPIO8_1 */
      Method (\_SB.GP02._STA) { Return (Zero) }
      Method (\_SB.GP02.GPIP._STA) { Return (Zero) }

      /* SMBUS_I2C5 */
      Method (\_SB.I205._STA) { Return (Zero) }

      /* SMBUS_I2C6 */
      Method (\_SB.I206._STA) { Return (Zero) }

      /* UART_S */
      Method (\_SB.SR02._STA) { Return (Zero) }
    }

    /* MUX1 */
    Scope (\_SB) {
      Method (\_SB.MX01._STA) { Return (Zero) }

      /* GPIO8_2 */
      Method (\_SB.GP03._STA) { Return (Zero) }
      Method (\_SB.GP03.GPIP._STA) { Return (Zero) }

      /* QSPI_2 */
      Method (\_SB.SP01._STA) { Return (Zero) }
    }

    /* MUX2 */
    Scope (\_SB) {
      Method (\_SB.MX02._STA) { Return (Zero) }

      /* GPIO16 */
      Method (\_SB.GP01._STA) { Return (Zero) }
      Method (\_SB.GP01.GPIP._STA) { Return (Zero) }

      /* ESPI */
      Method (\_SB.ESP0._STA) { Return (Zero) }
    }

    /* PCIe0 P0 */
    Method (\_SB.PC00._STA) { Return (Zero) }

    /* PCIe0 P1 */
    Method (\_SB.PC01._STA) { Return (Zero) }

    /* PCIe1 P0 */
    Method (\_SB.PC02._STA) { Return (Zero) }

    /* PCIe1 P1 */
    Method (\_SB.PC03._STA) { Return (Zero) }

    /* PCIe2 P0 */
    Method (\_SB.PC04._STA) { Return (0xF) }
    Name (\_SB.PC04.NUML, 16)

    /* PCIe2 P1 */
    Method (\_SB.PC05._STA) { Return (Zero) }

    /* PCIe3 P0 */
    Method (\_SB.PC06._STA) { Return (0xF) }
    Name (\_SB.PC06.NUML, 16)

    /* PCIe3 P1 */
    Method (\_SB.PC07._STA) { Return (Zero) }

    /* PCIe3 P2 */
    Method (\_SB.PC08._STA) { Return (Zero) }

    /* PCIe3 P3 */
    Method (\_SB.PC09._STA) { Return (Zero) }

    /* PCIe4 P0 */
    Method (\_SB.PC0A._STA) { Return (0xF) }
    Name (\_SB.PC0A.NUML, 4)

    /* PCIe4 P1 */
    Method (\_SB.PC0B._STA) { Return (0xF) }
    Name (\_SB.PC0B.NUML, 4)

    /* PCIe4 P2 */
    Method (\_SB.PC0C._STA) { Return (0xF) }
    Name (\_SB.PC0C.NUML, 4)

    /* PCIe4 P3 */
    Method (\_SB.PC0D._STA) { Return (0xF) }
    Name (\_SB.PC0D.NUML, 4)
  }

  /* Chip1 */
  BAIKAL_DSDT_CHIP_NODES(1)
  Scope (\_SB)
  {
    /* Cluster PVT */
    Method (\_SB.P100._STA) { Return (0xF) }
    Method (\_SB.P101._STA) { Return (0xF) }
    Method (\_SB.P102._STA) { Return (0xF) }
    Method (\_SB.P103._STA) { Return (0xF) }
    Method (\_SB.P104._STA) { Return (0xF) }
    Method (\_SB.P105._STA) { Return (0xF) }
    Method (\_SB.P106._STA) { Return (0xF) }
    Method (\_SB.P107._STA) { Return (0xF) }
    Method (\_SB.P108._STA) { Return (0xF) }
    Method (\_SB.P109._STA) { Return (0xF) }
    Method (\_SB.P10A._STA) { Return (0xF) }
    Method (\_SB.P10B._STA) { Return (0xF) }

    /* PCIe PVT */
    Method (\_SB.P10C._STA) { Return (0xF) }
    Method (\_SB.P10D._STA) { Return (0xF) }
    Method (\_SB.P10E._STA) { Return (0xF) }
    Method (\_SB.P10F._STA) { Return (0xF) }
    Method (\_SB.P110._STA) { Return (0xF) }

    /* DDR PVT */
    Method (\_SB.P111._STA) { Return (0xF) }
    Method (\_SB.P112._STA) { Return (0xF) }
    Method (\_SB.P113._STA) { Return (0xF) }
    Method (\_SB.P114._STA) { Return (0xF) }
    Method (\_SB.P115._STA) { Return (0xF) }
    Method (\_SB.P116._STA) { Return (0xF) }

    /* Timer1 */
    Method (\_SB.TM11._STA) { Return (0xF) }

    /* Timer2 */
    Method (\_SB.TM12._STA) { Return (0xF) }

    /* Timer3 */
    Method (\_SB.TM13._STA) { Return (0xF) }

    /* Timer4 */
    Method (\_SB.TM14._STA) { Return (0xF) }

    /* WDT */
    Method (\_SB.WDT1._STA) { Return (0xF) }

    /* DDR0 */
    Method (\_SB.DR10._STA) { Return (Zero) }

    /* DDR1 */
    Method (\_SB.DR11._STA) { Return (Zero) }

    /* DDR2 */
    Method (\_SB.DR12._STA) { Return (Zero) }

    /* DDR3 */
    Method (\_SB.DR13._STA) { Return (Zero) }

    /* DDR4 */
    Method (\_SB.DR14._STA) { Return (Zero) }

    /* DDR5 */
    Method (\_SB.DR15._STA) { Return (Zero) }

    /* USB OHCI */
    Method (\_SB.UB10._STA) { Return (0xF) }

    /* USB EHCI */
    Method (\_SB.UB11._STA) { Return (0xF) }

    /* QSPI_1 */
    Method (\_SB.SP10._STA) { Return (0xF) }

    /* GPIO32 */
    Method (\_SB.GP10._STA) { Return (0xF) }
    Method (\_SB.GP10.GPIP._STA) { Return (0xF) }

    /* SMBUS_I2C2 */
    Method (\_SB.I212._STA) { Return (0xF) }

    /* SMBUS_I2C3 */
    Method (\_SB.I213._STA) { Return (0xF) }

    /* SMBUS_I2C4 */
    Method (\_SB.I214._STA) { Return (0xF) }

    /* UART_A1 */
    Method (\_SB.SR10._STA) { Return (0xF) }

    /* UART_A2 */
    Method (\_SB.SR11._STA) { Return (Zero) }

    /* GMAC0 */
    Method (\_SB.GM10._STA) { Return (0xF) }
    Method (\_SB.GM10.GPHY._STA) { Return (0xF) }

    /* GMAC1 */
    Method (\_SB.GM11._STA) { Return (0xF) }
    Method (\_SB.GM11.GPHY._STA) { Return (0xF) }

    /* MUX */
    Method (\_SB.MXC1._STA) { Return (Zero) }

    /* MUX0 */
    Scope (\_SB) {
      Method (\_SB.MX10._STA) { Return (Zero) }

      /* GPIO8_1 */
      Method (\_SB.GP12._STA) { Return (Zero) }
      Method (\_SB.GP12.GPIP._STA) { Return (Zero) }

      /* SMBUS_I2C5 */
      Method (\_SB.I215._STA) { Return (Zero) }

      /* SMBUS_I2C6 */
      Method (\_SB.I216._STA) { Return (Zero) }

      /* UART_S */
      Method (\_SB.SR12._STA) { Return (Zero) }
    }

    /* MUX1 */
    Scope (\_SB) {
      Method (\_SB.MX11._STA) { Return (Zero) }

      /* GPIO8_2 */
      Method (\_SB.GP13._STA) { Return (Zero) }
      Method (\_SB.GP13.GPIP._STA) { Return (Zero) }

      /* QSPI_2 */
      Method (\_SB.SP11._STA) { Return (Zero) }
    }

    /* MUX2 */
    Scope (\_SB) {
      Method (\_SB.MX12._STA) { Return (Zero) }

      /* GPIO16 */
      Method (\_SB.GP11._STA) { Return (Zero) }
      Method (\_SB.GP11.GPIP._STA) { Return (Zero) }

      /* ESPI */
      Method (\_SB.ESP1._STA) { Return (Zero) }
    }

    /* PCIe0 P0 */
    Method (\_SB.PC10._STA) { Return (Zero) }

    /* PCIe0 P1 */
    Method (\_SB.PC11._STA) { Return (Zero) }

    /* PCIe1 P0 */
    Method (\_SB.PC12._STA) { Return (Zero) }

    /* PCIe1 P1 */
    Method (\_SB.PC13._STA) { Return (Zero) }

    /* PCIe2 P0 */
    Method (\_SB.PC14._STA) { Return (0xF) }
    Name (\_SB.PC14.NUML, 16)

    /* PCIe2 P1 */
    Method (\_SB.PC15._STA) { Return (Zero) }

    /* PCIe3 P0 */
    Method (\_SB.PC16._STA) { Return (0xF) }
    Name (\_SB.PC16.NUML, 16)

    /* PCIe3 P1 */
    Method (\_SB.PC17._STA) { Return (Zero) }

    /* PCIe3 P2 */
    Method (\_SB.PC18._STA) { Return (Zero) }

    /* PCIe3 P3 */
    Method (\_SB.PC19._STA) { Return (Zero) }

    /* PCIe4 P0 */
    Method (\_SB.PC1A._STA) { Return (0xF) }
    Name (\_SB.PC1A.NUML, 4)

    /* PCIe4 P1 */
    Method (\_SB.PC1B._STA) { Return (0xF) }
    Name (\_SB.PC1B.NUML, 4)

    /* PCIe4 P2 */
    Method (\_SB.PC1C._STA) { Return (0xF) }
    Name (\_SB.PC1C.NUML, 4)

    /* PCIe4 P3 */
    Method (\_SB.PC1D._STA) { Return (0xF) }
    Name (\_SB.PC1D.NUML, 4)
  }

  /* Additional devices */
  Scope (\_SB)
  {
    Device (\_SB.I204.PR6F)
    {
      Name (_HID, "PRP0001")
      Name (_UID, PLATFORM_ADDR_OUT_CHIP(0, 0x00CB006F))
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        I2CSerialBusV2 (0x6F, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I204")
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "microchip,mcp7940x" }
        }
      })
    }

    Device (\_SB.SP00.PR00)
    {
      Name (_HID, "PRP0001")
      Name (_UID, PLATFORM_ADDR_OUT_CHIP(0, 0x00C20000))
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        SPISerialBusV2 (Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 1000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.SP00")
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "baikal,partitions", Package ()
          {
            "BL1",       0x000000, 0x020000,
            "DDR-FW",    0x020000, 0x020000,
            "FIP",       0x040000, 0x200000,
            "UEFI vars", 0x2C0000, 0x0C0000
          }}
        }
      })
    }

    Device (\_SB.SP10.PR00)
    {
      Name (_HID, "PRP0001")
      Name (_UID, PLATFORM_ADDR_OUT_CHIP(1, 0x00C20000))
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        SPISerialBusV2 (Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 1000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.SP10")
      })
    }
  }
}
