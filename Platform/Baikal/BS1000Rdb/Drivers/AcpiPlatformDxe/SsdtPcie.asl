/** @file
  Copyright (c) 2022, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <BS1000.h>

#include "AcpiPlatform.h"

#define BUS_RES WordBusNumber (ResourceProducer, MinFixed, MaxFixed,, 0x0, 0x1, 0xFE, 0x0, 0xFE)

DefinitionBlock (__FILE__, "SSDT", 2, "BAIKAL", "SSDTPCI0", 1)
{
  Scope (_SB_)
  {
    Device (PCI0)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_P0_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE0_P0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE0_P0_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE0_P0_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE0_P0_CFG_BASE,
                         BAIKAL_ACPI_PCIE0_P0_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE0_P0_DBI_BASE, BS1000_PCIE0_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE0_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 177, 179 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

#ifdef BAIKAL_ACPI_PCIE0_P1_SEGMENT
    Device (PCI1)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_P1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE0_P1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE0_P1_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE0_P1_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE0_P1_CFG_BASE,
                         BAIKAL_ACPI_PCIE0_P1_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE0_P1_DBI_BASE, BS1000_PCIE0_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE0_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 178, 180 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }
#endif

    Device (PCI2)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE1_P0_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE1_P0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE1_P0_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE1_P0_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE1_P0_CFG_BASE,
                         BAIKAL_ACPI_PCIE1_P0_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE1_P0_DBI_BASE, BS1000_PCIE1_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE1_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 220, 222 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

#ifdef BAIKAL_ACPI_PCIE1_P1_SEGMENT
    Device (PCI3)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE1_P1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE1_P1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE1_P1_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE1_P1_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE1_P1_CFG_BASE,
                         BAIKAL_ACPI_PCIE1_P1_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE1_P1_DBI_BASE, BS1000_PCIE1_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE1_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 221, 223 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }
#endif

    Device (PCI4)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE2_P0_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE2_P0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE2_P0_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE2_P0_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE2_P0_CFG_BASE,
                         BAIKAL_ACPI_PCIE2_P0_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE2_P0_DBI_BASE, BS1000_PCIE2_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE2_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 134, 136 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI5)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE2_P1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE2_P1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE2_P1_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE2_P1_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 8)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE2_P1_CFG_BASE,
                         BAIKAL_ACPI_PCIE2_P1_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE2_P1_DBI_BASE, BS1000_PCIE2_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE2_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 135, 137 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

#ifdef BAIKAL_ACPI_PCIE3_P0_SEGMENT
    Device (PCI6)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P0_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE3_P0_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE3_P0_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE3_P0_CFG_BASE,
                         BAIKAL_ACPI_PCIE3_P0_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P0_DBI_BASE, BS1000_PCIE3_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 281, 285 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }
#endif

    Device (PCI7)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE3_P1_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE3_P1_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE3_P1_CFG_BASE,
                         BAIKAL_ACPI_PCIE3_P1_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P1_DBI_BASE, BS1000_PCIE3_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 282, 286 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI8)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P2_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P2_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE3_P2_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE3_P2_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE3_P2_CFG_BASE,
                         BAIKAL_ACPI_PCIE3_P2_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P2_DBI_BASE, BS1000_PCIE3_P2_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P2_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 283, 287 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCI9)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P3_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P3_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE3_P3_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE3_P3_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE3_P3_CFG_BASE,
                         BAIKAL_ACPI_PCIE3_P3_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P3_DBI_BASE, BS1000_PCIE3_P3_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P3_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 284, 288 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCIA)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE4_P0_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE4_P0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE4_P0_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE4_P0_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE4_P0_CFG_BASE,
                         BAIKAL_ACPI_PCIE4_P0_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P0_DBI_BASE, BS1000_PCIE4_P0_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 350, 354 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

#ifdef BAIKAL_ACPI_PCIE4_P1_SEGMENT
    Device (PCIB)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE4_P1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE4_P1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE4_P1_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE4_P1_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE4_P1_CFG_BASE,
                         BAIKAL_ACPI_PCIE4_P1_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P1_DBI_BASE, BS1000_PCIE4_P1_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 351, 355 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }
#endif

    Device (PCIC)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE4_P2_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE4_P2_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE4_P2_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE4_P2_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE4_P2_CFG_BASE,
                         BAIKAL_ACPI_PCIE4_P2_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P2_DBI_BASE, BS1000_PCIE4_P2_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P2_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 352, 356 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }

    Device (PCID)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE4_P3_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE4_P3_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
        Return (0xF)
      }

      Name (_PRT, Package()
      {
        Package() { 0x0000FFFF, 0, Zero, Zero },
        Package() { 0x0000FFFF, 1, Zero, Zero },
        Package() { 0x0000FFFF, 2, Zero, Zero },
        Package() { 0x0000FFFF, 3, Zero, Zero }
      })

      Method (_CRS, 0, Serialized)
      {
        Store (ResourceTemplate ()
        {
          BUS_RES
          QWordMemory (ResourceProducer,, MinFixed, MaxFixed,,, 0x0,
                       BAIKAL_ACPI_PCIE_MEM_BASE,
                       BAIKAL_ACPI_PCIE_MEM_MAX,
                       BAIKAL_ACPI_PCIE4_P3_MEM_OFFSET,
                       BAIKAL_ACPI_PCIE_MEM_SIZE)
          QWordIO (ResourceProducer, MinFixed, MaxFixed,, EntireRange, 0x0,
                   BAIKAL_ACPI_PCIE_IO_BASE,
                   BAIKAL_ACPI_PCIE_IO_MAX,
                   BAIKAL_ACPI_PCIE4_P3_IO_OFFSET,
                   BAIKAL_ACPI_PCIE_IO_SIZE,,,,
                   TypeTranslation)
        }, Local0)

        Return (Local0)
      }

      Name (NUML, 4)
      Name (NUMV, 4)

      Device (RES0)
      {
        Name (_ADR, Zero)
        Method (_CRS, 0, Serialized)
        {
          Store (ResourceTemplate ()
          {
            QWordMemory (,, MinFixed, MaxFixed,,, 0x0,
                         BAIKAL_ACPI_PCIE4_P3_CFG_BASE,
                         BAIKAL_ACPI_PCIE4_P3_CFG_MAX,
                         BAIKAL_ACPI_PCIE_CFG_OFFSET,
                         BAIKAL_ACPI_PCIE_CFG_SIZE)
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P3_DBI_BASE, BS1000_PCIE4_P3_DBI_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P3_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 353, 357 }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC
    }
  }
}
