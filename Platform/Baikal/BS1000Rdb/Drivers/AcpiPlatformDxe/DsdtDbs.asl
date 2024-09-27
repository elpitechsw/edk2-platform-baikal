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
#define P010_INTERRUPT_ENABLE  1
#define P011_INTERRUPT_ENABLE  1
#define P012_INTERRUPT_ENABLE  1
#define P013_INTERRUPT_ENABLE  1
#define P014_INTERRUPT_ENABLE  1
#define P015_INTERRUPT_ENABLE  1
#define P016_INTERRUPT_ENABLE  1

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
    Method (\_SB.SR01._STA) { Return (0xF) }

    /* GMAC0 */
    Method (\_SB.GM00._STA) { Return (0xF) }
    Method (\_SB.GM00.GPHY._STA) { Return (0xF) }

    /* GMAC1 */
    Method (\_SB.GM01._STA) { Return (0xF) }
    Method (\_SB.GM01.GPHY._STA) { Return (0xF) }

    /* MUX */
    Method (\_SB.MXC0._STA) { Return (0xF) }

    /* MUX0 */
    Scope (\_SB) {
      Method (\_SB.MX00._STA) { Return (0xF) }

      /* GPIO8_1 */
      Method (\_SB.GP02._STA) { Return (\_SB.MX00.STA0) }
      Method (\_SB.GP02.GPIP._STA) { Return (\_SB.MX00.STA0) }

      /* SMBUS_I2C5 */
      Method (\_SB.I205._STA) { Return (\_SB.MX00.STA1) }

      /* SMBUS_I2C6 */
      Method (\_SB.I206._STA) { Return (\_SB.MX00.STA1) }

      /* UART_S */
      Method (\_SB.SR02._STA) { Return (\_SB.MX00.STA1) }
    }

    /* MUX1 */
    Scope (\_SB) {
      Method (\_SB.MX01._STA) { Return (0xF) }

      /* GPIO8_2 */
      Method (\_SB.GP03._STA) { Return (\_SB.MX01.STA0) }
      Method (\_SB.GP03.GPIP._STA) { Return (\_SB.MX01.STA0) }

      /* QSPI_2 */
      Method (\_SB.SP01._STA) { Return (\_SB.MX01.STA1) }
    }

    /* MUX2 */
    Scope (\_SB) {
      Method (\_SB.MX02._STA) { Return (0xF) }

      /* GPIO16 */
      Method (\_SB.GP01._STA) { Return (\_SB.MX02.STA0) }
      Method (\_SB.GP01.GPIP._STA) { Return (\_SB.MX02.STA0) }

      /* ESPI */
      Method (\_SB.ESP0._STA) { Return (\_SB.MX02.STA1) }
    }

    /* PCIe0 P0 */
    Method (\_SB.PC00._STA) { Return (0xF) }
    Name (\_SB.PC00.NUML, 8)

    /* PCIe0 P1 */
    Method (\_SB.PC01._STA) { Return (0xF) }
    Name (\_SB.PC01.NUML, 8)

    /* PCIe1 P0 */
    Method (\_SB.PC02._STA) { Return (0xF) }
    Name (\_SB.PC02.NUML, 8)

    /* PCIe1 P1 */
    Method (\_SB.PC03._STA) { Return (0xF) }
    Name (\_SB.PC03.NUML, 8)

    /* PCIe2 P0 */
    Method (\_SB.PC04._STA) { Return (0xF) }
    Name (\_SB.PC04.NUML, 8)

    /* PCIe2 P1 */
    Method (\_SB.PC05._STA) { Return (0xF) }
    Name (\_SB.PC05.NUML, 8)

    /* PCIe3 P0 */
    Method (\_SB.PC06._STA) { Return (0xF) }
    Name (\_SB.PC06.NUML, 4)

    /* PCIe3 P1 */
    Method (\_SB.PC07._STA) { Return (0xF) }
    Name (\_SB.PC07.NUML, 4)

    /* PCIe3 P2 */
    Method (\_SB.PC08._STA) { Return (0xF) }
    Name (\_SB.PC08.NUML, 4)

    /* PCIe3 P3 */
    Method (\_SB.PC09._STA) { Return (0xF) }
    Name (\_SB.PC09.NUML, 4)

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

  /* Additional devices */
  Scope (\_SB)
  {
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

    /* Include ("EspiMezz.asl") */
    /* Include ("Qspi2Mezz.asl") */
  }
}
