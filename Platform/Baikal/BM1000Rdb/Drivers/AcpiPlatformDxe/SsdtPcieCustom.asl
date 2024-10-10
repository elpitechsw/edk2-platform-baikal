/** @file
  Copyright (c) 2020 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <BM1000.h>

#include "AcpiPlatform.h"

#define BUS_RES WordBusNumber (ResourceProducer, MinFixed, MaxFixed,, 0x0, 0x1, 0xFE, 0x0, 0xFE)

#define ACPI_BAIKAL_PWM_PCI_CLK(Channel0, Channel1, Channel2) \
  ACPI_BAIKAL_SMC_CMU_DATA                                    \
  PowerResource (PWRR, 0, 0)                                  \
  {                                                           \
    Name (PWRV, 1)                                            \
    Method (_STA)                                             \
    {                                                         \
      Return (PWRV)                                           \
    }                                                         \
    Method (_ON)                                              \
    {                                                         \
      ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x02000000, Channel0)     \
      ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x02000000, Channel1)     \
      ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x02000000, Channel2)     \
      PWRV = 1                                                \
    }                                                         \
    Method (_OFF)                                             \
    {                                                         \
      ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x02000000, Channel2)    \
      ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x02000000, Channel1)    \
      ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x02000000, Channel0)    \
      PWRV = 0                                                \
    }                                                         \
  }                                                           \
  Name (_PR0, Package () { PWRR })                            \
  Name (_PR3, Package () { PWRR })                            \
  ACPI_BAIKAL_PWM_PS_METHODS

DefinitionBlock (__FILE__, "SSDT", 2, "BAIKAL", "SSDTPCI0", 1)
{
  External (\_SB.CSTA, IntObj)
  External (\_SB.CRU0, DeviceObj)
  External (\_SB.GPIO, DeviceObj)
  External (\_SB.GPIO.GPIP, DeviceObj)

  Scope (_SB_)
  {
    // PCIe0 (x4 #0)
    Device (PCI0)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Ones },
        Package() { 0x0000FFFF, 1, Zero, Ones },
        Package() { 0x0000FFFF, 2, Zero, Ones },
        Package() { 0x0000FFFF, 3, Zero, Ones }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE0_MEM_BASE,
                       BAIKAL_ACPI_PCIE0_MEM_MAX,
                       BAIKAL_ACPI_PCIE0_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE0_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE0_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "bm1000,gen3-eq-fb-mode", 0 },
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
          Package () { "bm1000,phy-rx-ctle", 0xF4 },
#else
          Package () { "bm1000,phy-rx-ctle", 0x34 },
#endif
          Package () { "bm1000,phy-rx-dfe", 0 }
        }
      })

      Name (NUML, 4)
      Name (NUMV, 4)

//      ACPI_BAIKAL_PWM_PCI_CLK (0, 1, 2)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE0_CFG_BASE,
                         BAIKAL_ACPI_PCIE0_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BM1000_PCIE0_DBI_BASE, BM1000_PCIE0_DBI_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 458, 461 }
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
            GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 6 }
#endif
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

#ifdef BAIKAL_ACPI_PCIE1_SEGMENT
    // PCIe1 (x4 #1)
    Device (PCI1)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Ones },
        Package() { 0x0000FFFF, 1, Zero, Ones },
        Package() { 0x0000FFFF, 2, Zero, Ones },
        Package() { 0x0000FFFF, 3, Zero, Ones }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE1_MEM_BASE,
                       BAIKAL_ACPI_PCIE1_MEM_MAX,
                       BAIKAL_ACPI_PCIE1_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE1_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE1_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "bm1000,gen3-eq-fb-mode", 0 },
          Package () { "bm1000,phy-rx-ctle", 0x64 },
          Package () { "bm1000,phy-rx-dfe", 0 }
        }
      })

      Name (NUML, 4)
      Name (NUMV, 4)

//      ACPI_BAIKAL_PWM_PCI_CLK (3, 4, 5)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE1_CFG_BASE,
                         BAIKAL_ACPI_PCIE1_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BM1000_PCIE1_DBI_BASE, BM1000_PCIE1_DBI_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 434, 437 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }
#endif

#ifdef BAIKAL_ACPI_PCIE2_SEGMENT
    // PCIe2 (x8)
    Device (PCI2)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE2_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE2_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Ones },
        Package() { 0x0000FFFF, 1, Zero, Ones },
        Package() { 0x0000FFFF, 2, Zero, Ones },
        Package() { 0x0000FFFF, 3, Zero, Ones }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE2_MEM_BASE,
                       BAIKAL_ACPI_PCIE2_MEM_MAX,
                       BAIKAL_ACPI_PCIE2_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE2_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE2_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "bm1000,gen3-eq-fb-mode", 0 },
          Package () { "bm1000,phy-rx-ctle", 0x34 },
          Package () { "bm1000,phy-rx-dfe", 0 }
        }
      })

      Name (NUML, 8)
      Name (NUMV, 16)

//      ACPI_BAIKAL_PWM_PCI_CLK (6, 7, 8)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE2_CFG_BASE,
                         BAIKAL_ACPI_PCIE2_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BM1000_PCIE2_DBI_BASE, BM1000_PCIE2_DBI_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 410, 413 }
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
            GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 3, 1 }
#endif
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }
#endif
  }
}
