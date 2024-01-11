/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiPlatform.h"

#include <BS1000.h>

#define BUS_RES WordBusNumber (ResourceProducer, MinFixed, MaxFixed,, 0x0, 0x1, 0xFE, 0x0, 0xFE)

DefinitionBlock (__FILE__, "SSDT", 2, "BAIKAL", "SSDTPCI0", 1)
{
  Scope (_SB_)
  {
    Device (PC00)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_P0_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE0_P0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (0xF)
#endif
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

#if defined(BAIKAL_MBS_1S) || defined(BAIKAL_MBS_2S)
      Name (NUML, 16)
#else
      Name (NUML, 8)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE0_P0_ATU_BASE, BS1000_PCIE0_P0_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE0_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              161, 162, 163, 164, 165, 166, 167, 168,
              177, 179
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

#ifdef BAIKAL_ACPI_PCIE0_P1_SEGMENT
    Device (PC01)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_P1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE0_P1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
#if defined(BAIKAL_MBS_1S) || defined(BAIKAL_MBS_2S)
        Return (Zero)
#else
        Return (0xF)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE0_P1_ATU_BASE, BS1000_PCIE0_P1_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE0_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            { 169, 170, 171, 172, 173, 174, 175, 176,
              178, 180
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }
#endif

    Device (PC02)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE1_P0_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE1_P0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (0xF)
#endif
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

#if defined(BAIKAL_MBS_1S) || defined(BAIKAL_MBS_2S)
      Name (NUML, 16)
#else
      Name (NUML, 8)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE1_P0_ATU_BASE, BS1000_PCIE1_P0_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE1_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              204, 205, 206, 207, 208, 209, 210, 211,
              220, 222
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

#ifdef BAIKAL_ACPI_PCIE1_P1_SEGMENT
    Device (PC03)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE1_P1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE1_P1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
#if defined(BAIKAL_MBS_1S) || defined(BAIKAL_MBS_2S)
        Return (Zero)
#else
        Return (0xF)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE1_P1_ATU_BASE, BS1000_PCIE1_P1_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE1_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              212, 213, 214, 215, 216, 217, 218, 219,
              221, 223
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }
#endif

    Device (PC04)
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

#ifdef BAIKAL_MBS_2S
      Name (NUML, 16)
#else
      Name (NUML, 8)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE2_P0_ATU_BASE, BS1000_PCIE2_P0_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE2_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              118, 119, 120, 121, 122, 123, 124, 125,
              134, 136
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

    Device (PC05)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE2_P1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE2_P1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (0xF)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE2_P1_ATU_BASE, BS1000_PCIE2_P1_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE2_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              126, 127, 128, 129, 130, 131, 132, 133,
              135, 137
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

#ifdef BAIKAL_ACPI_PCIE3_P0_SEGMENT
    Device (PC06)
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

#if defined(BAIKAL_MBS_1S) || defined(BAIKAL_MBS_2S)
      Name (NUML, 16)
#else
      Name (NUML, 4)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P0_ATU_BASE, BS1000_PCIE3_P0_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              249, 250, 251, 252, 253, 254, 255, 256,
              281, 285
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }
#endif

    Device (PC07)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P1_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
#if defined(BAIKAL_MBS_1S) || defined(BAIKAL_MBS_2S)
        Return (Zero)
#else
        Return (0xF)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P1_ATU_BASE, BS1000_PCIE3_P1_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              257, 258, 259, 260, 261, 262, 263, 264,
              282, 286
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

    Device (PC08)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P2_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P2_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
#if defined(BAIKAL_MBS_1S) || defined(BAIKAL_MBS_2S)
        Return (Zero)
#else
        Return (0xF)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P2_ATU_BASE, BS1000_PCIE3_P2_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P2_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              265, 266, 267, 268, 269, 270, 271, 272,
              283, 287
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

    Device (PC09)
    {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE3_P3_SEGMENT)
      Name (_CCA, Zero)
      Name (_SEG, BAIKAL_ACPI_PCIE3_P3_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA)
      {
#if defined(BAIKAL_MBS_1S) || defined(BAIKAL_MBS_2S)
        Return (Zero)
#else
        Return (0xF)
#endif
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
            Memory32Fixed (ReadWrite, BS1000_PCIE3_P3_ATU_BASE, BS1000_PCIE3_P3_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE3_P3_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              273, 274, 275, 276, 277, 278, 279, 280,
              284, 288
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

    Device (PC10)
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
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P0_ATU_BASE, BS1000_PCIE4_P0_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P0_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              318, 319, 320, 321, 322, 323, 324, 325,
              350, 354
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

#ifdef BAIKAL_ACPI_PCIE4_P1_SEGMENT
    Device (PC11)
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
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P1_ATU_BASE, BS1000_PCIE4_P1_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P1_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              326, 327, 328, 329, 330, 331, 332, 333,
              351, 355
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }
#endif

    Device (PC12)
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
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P2_ATU_BASE, BS1000_PCIE4_P2_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P2_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              334, 335, 336, 337, 338, 339, 340, 341,
              352, 356
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

    Device (PC13)
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
            Memory32Fixed (ReadWrite, BS1000_PCIE4_P3_ATU_BASE, BS1000_PCIE4_P3_ATU_SIZE)
            Memory32Fixed (ReadWrite, BAIKAL_ACPI_PCIE4_P3_APB_BASE, BAIKAL_ACPI_PCIE_APB_SIZE)
            Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
            {
              342, 343, 344, 345, 346, 347, 348, 349,
              353, 357
            }
          }, Local0)

          Return (Local0)
        }
      }

      NATIVE_PCIE_OSC

      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

#if (PLATFORM_CHIP_COUNT > 1)
    Include("SsdtPcie-S1.asi")
#endif
  }
}
