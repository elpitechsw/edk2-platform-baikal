/** @file
  Copyright (c) 2021 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiPlatform.h"

#include <BS1000.h>

#define BAIKAL_DSDT_CPU_NODE(Id, CpuId) \
  Device (CPU##Id)                      \
  {                                     \
    Name (_HID, "ACPI0007")             \
    Name (_UID, CpuId)                  \
    Method (_STA)                       \
    {                                   \
      Return (0xF)                      \
    }                                   \
  }

#define BAIKAL_DSDT_CLUSTER_NODE(Id, ClusterId, CpuId0, CpuId1, CpuId2, CpuId3) \
  Device (CL##Id)                                                               \
  {                                                                             \
    Name (_HID, "ACPI0010")                                                     \
    Name (_UID, BAIKAL_ACPI_CLUSTER_ID(ClusterId))                              \
    Method (_STA)                                                               \
    {                                                                           \
      Return (0xF)                                                              \
    }                                                                           \
    BAIKAL_DSDT_CPU_NODE (0, CpuId0)                                            \
    BAIKAL_DSDT_CPU_NODE (1, CpuId1)                                            \
    BAIKAL_DSDT_CPU_NODE (2, CpuId2)                                            \
    BAIKAL_DSDT_CPU_NODE (3, CpuId3)                                            \
  }

DefinitionBlock ("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1)
{
  Scope (_SB_)
  {
    /* Chip 0 CPUs */
    BAIKAL_DSDT_CLUSTER_NODE (00, 0, 0, 1, 2, 3)
    BAIKAL_DSDT_CLUSTER_NODE (01, 1, 4, 5, 6, 7)
    BAIKAL_DSDT_CLUSTER_NODE (02, 2, 8, 9, 10, 11)
    BAIKAL_DSDT_CLUSTER_NODE (03, 3, 12, 13, 14, 15)
    BAIKAL_DSDT_CLUSTER_NODE (04, 4, 16, 17, 18, 19)
    BAIKAL_DSDT_CLUSTER_NODE (05, 5, 20, 21, 22, 23)
    BAIKAL_DSDT_CLUSTER_NODE (06, 6, 24, 25, 26, 27)
    BAIKAL_DSDT_CLUSTER_NODE (07, 7, 28, 29, 30, 31)
    BAIKAL_DSDT_CLUSTER_NODE (08, 8, 32, 33, 34, 35)
    BAIKAL_DSDT_CLUSTER_NODE (09, 9, 36, 37, 38, 39)
    BAIKAL_DSDT_CLUSTER_NODE (10, 10, 40, 41, 42, 43)
    BAIKAL_DSDT_CLUSTER_NODE (11, 11, 44, 45, 46, 47)

    Device (CLK0)
    {
      Name (_HID, "BKLE1001")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      /* CMU addr, clock name */
      Name (PROP, Package ()
      {
        0x00410000, "socket0_sc_cmu1"
      })
      /* Device reference, clock name, clock id, con_id */
      Name (CLKS, Package ()
      {
        ^GMC0, "gmac1_apb", 9, "stmmaceth",
        ^GMC0, "gmac1_axi", 10, "axi_clk",
        ^GMC1, "gmac2_apb", 11, "stmmaceth",
        ^GMC1, "gmac2_axi", 12, "axi_clk"
      })
    }

    Device (CLK1)
    {
      Name (_HID, "BKLE1001")
      Name (_UID, One)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      /* CMU addr, clock name */
      Name (PROP, Package ()
      {
        0x00420000, "socket0_sc_cmu2"
      })
      /* Device reference, clock name, clock id, con_id */
      Name (CLKS, Package ()
      {
        ^GMC0, "gmac1_ptp", 1, "ptp_ref",
        ^GMC0, "gmac1_txx2", 2, "tx2_clk",
        ^GMC1, "gmac2_ptp", 3, "ptp_ref",
        ^GMC1, "gmac2_txx2", 4, "tx2_clk"
      })
    }

    // PVT_CLUSTER0
    Device (PV00)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x04030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x04030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER1
    Device (PV01)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x08030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x08030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER2
    Device (PV02)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x0C030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x0C030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER3
    Device (PV03)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x10030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x10030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER4
    Device (PV04)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x14030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x14030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER5
    Device (PV05)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x18030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x18030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER6
    Device (PV06)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x1C030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x1C030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER7
    Device (PV07)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x20030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x20030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER8
    Device (PV08)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x24030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x24030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER9
    Device (PV09)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x28030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x28030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER10
    Device (PV10)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x2C030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x2C030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_CLUSTER11
    Device (PV11)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x30030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x30030000, 0x1000)
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE0
    Device (PV12)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x38030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x38030000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 195 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE1
    Device (PV13)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x3C030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x3C030000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 238 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE2
    Device (PV14)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x44030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x44030000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 152 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE3
    Device (PV15)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x48030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x48030000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 307 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_PCIE4
    Device (PV16)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x4C030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x4C030000, 0x1000)
#ifndef BAIKAL_MBS_2S
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 376 }
#endif
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR0
    Device (PV17)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x50030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x50030000, 0x1000)
#ifndef BAIKAL_MBS_2S
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 395 }
#endif
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR1
    Device (PV18)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x54030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x54030000, 0x1000)
#ifndef BAIKAL_MBS_2S
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 412 }
#endif
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR2
    Device (PV19)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x58030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x58030000, 0x1000)
#ifndef BAIKAL_MBS_2S
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 429 }
#endif
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR3
    Device (PV20)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x60030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x60030000, 0x1000)
#ifndef BAIKAL_MBS_2S
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 446 }
#endif
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR4
    Device (PV21)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x64030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x64030000, 0x1000)
#ifndef BAIKAL_MBS_2S
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 463 }
#endif
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // PVT_DDR5
    Device (PV22)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x68030000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x68030000, 0x1000)
#ifndef BAIKAL_MBS_2S
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 480 }
#endif
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-pvt" }
        }
      })
    }

    // USB OHCI
    Device (USB0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x00A00000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00A00000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 110 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "generic-ohci" }
        }
      })
    }

    // USB EHCI
    Device (USB1)
    {
      Name (_HID, "PNP0D20")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00A10000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 114 }
      })
    }

    // QSPI_1
    Device (QSP0)
    {
      Name (_HID, "HISI0173")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C20000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 95 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "num-cs", 4 }
        }
      })

      Device (PR00)
      {
        Name (_ADR, Zero)
        Name (_UID, 0x00C20000)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          SPISerialBusV2 (Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 10000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.QSP0")
        })
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "baikal,partitions", Package ()
            {
#ifdef BAIKAL_MBS_2S
              "bl1",       0x000000, 0x040000,
              "dtb",       0x040000, 0x0A0000,
              "trfw",      0x0E0000, 0x020000,
              "uefi-vars", 0x100000, 0x0C0000,
              "fip",       0x1C0000, 0x640000
#else
              "bl1",       0x000000, 0x040000,
              "dtb",       0x040000, 0x020000,
              "trfw",      0x060000, 0x020000,
              "uefi-vars", 0x080000, 0x0C0000,
              "fip",       0x140000, 0x6C0000
#endif
            }}
          }
        })
      }
    }

    // QSPI_2
    Device (QSP1)
    {
      Name (_HID, "HISI0173")
      Name (_UID, One)
      Name (_CCA, Zero)
      Method (_STA, 0, Serialized)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (\_SB.MUX1.STA1)
#endif
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C30000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 96 }
      })
    }

    // ESPI
    Device (ESP0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x00C40000)
      Name (_CCA, Zero)
      Method (_STA, 0, Serialized)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (\_SB.MUX2.STA1)
#endif
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C40000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 102 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-espi" }
        }
      })
    }

    // GPIO32
    Device (GP00)
    {
      Name (_HID, "APMC0D07")
      Name (_UID, 0x00C50000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C50000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 86 }
      })

      // GPIO port
      Device (GPIP)
      {
        Name (_ADR, Zero)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "reg", Zero },
            Package () { "snps,nr-gpios", 32 }
          }
        })
      }
    }

    // GPIO16
    Device (GP01)
    {
      Name (_HID, "APMC0D07")
      Name (_UID, 0x00C60000)
      Name (_CCA, Zero)
      Method (_STA, 0, Serialized)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (\_SB.MUX2.STA0)
#endif
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C60000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 83 }
      })

      // GPIO port
      Device (GPIP)
      {
        Name (_ADR, Zero)
        Method (_STA, 0, Serialized)
        {
          Return (\_SB.MUX2.STA0)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "reg", Zero },
            Package () { "snps,nr-gpios", 16 }
          }
        })
      }
    }

    // GPIO8_1
    Device (GP02)
    {
      Name (_HID, "APMC0D07")
      Name (_UID, 0x00C70000)
      Name (_CCA, Zero)
      Method (_STA, 0, Serialized)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (\_SB.MUX0.STA0)
#endif
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C70000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 84 }
      })

      // GPIO port
      Device (GPIP)
      {
        Name (_ADR, Zero)
        Method (_STA, 0, Serialized)
        {
          Return (\_SB.MUX0.STA0)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "reg", Zero },
            Package () { "snps,nr-gpios", 8 }
          }
        })
      }
    }

    // GPIO8_2
    Device (GP03)
    {
      Name (_HID, "APMC0D07")
      Name (_UID, 0x00C80000)
      Name (_CCA, Zero)
      Method (_STA, 0, Serialized)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (\_SB.MUX1.STA0)
#endif
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C80000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 85 }
      })

      // GPIO port
      Device (GPIP)
      {
        Name (_ADR, Zero)
        Method (_STA, 0, Serialized)
        {
          Return (\_SB.MUX1.STA0)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "reg", Zero },
            Package () { "snps,nr-gpios", 8 }
          }
        })
      }
    }

    // SMBUS_I2C2
    Device (I202)
    {
      Name (_HID, "APMC0D0F")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C90000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 97 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "i2c-sda-hold-time-ns", 500 },
          Package () { "clock-frequency", 400000 }
        }
      })
    }

    // SMBUS_I2C3
    Device (I203)
    {
      Name (_HID, "APMC0D0F")
      Name (_UID, One)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00CA0000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 98 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "i2c-sda-hold-time-ns", 500 },
          Package () { "clock-frequency", 400000 }
        }
      })
    }

    // SMBUS_I2C4
    Device (I204)
    {
      Name (_HID, "APMC0D0F")
      Name (_UID, 2)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00CB0000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 99 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "i2c-sda-hold-time-ns", 500 },
          Package () { "clock-frequency", 400000 }
        }
      })
    }

    // SMBUS_I2C5
    Device (I205)
    {
      Name (_HID, "APMC0D0F")
      Name (_UID, 3)
      Name (_CCA, Zero)
      Method (_STA, 0, Serialized)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (\_SB.MUX0.STA1)
#endif
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00CC0000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 100 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "i2c-sda-hold-time-ns", 500 },
          Package () { "clock-frequency", 400000 }
        }
      })
    }

    // SMBUS_I2C6
    Device (I206)
    {
      Name (_HID, "APMC0D0F")
      Name (_UID, 4)
      Name (_CCA, Zero)
      Method (_STA, 0, Serialized)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (\_SB.MUX0.STA1)
#endif
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00CD0000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 101 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "i2c-sda-hold-time-ns", 500 },
          Package () { "clock-frequency", 400000 }
        }
      })
    }

    // UART_A1
    Device (COM0)
    {
      Name (_HID, "ARMH0011")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C00000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 93 }
      })
    }

    // UART_A2
    Device (COM1)
    {
      Name (_HID, "ARMH0011")
      Name (_UID, One)
      Name (_CCA, Zero)
      Method (_STA)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (0xF)
#endif
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00C10000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 94 }
      })
    }

    // UART_S
    Device (COM2)
    {
      Name (_HID, "HISI0031")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA, 0, Serialized)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (\_SB.MUX0.STA1)
#endif
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00E00000, 0x100)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 92 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "reg-shift", 2 },
          Package () { "reg-io-width", 4 },
          Package () { "clock-frequency", 7372800 }
        }
      })
    }

    // GMAC0
    Device (GMC0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x00A20000)
      Name (_CCA, One)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00A20000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 108 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-gmac" },
          Package () { "max-speed", 1000 },
          Package () { "reg", One },
          Package () { "phy-mode", "rgmii-rxid" },
        }
      })

      Device (GPHY)
      {
        Name (_ADR, Zero)
        Method (_STA)
        {
          Return (0xF)
        }
      }
    }

    // GMAC1
    Device (GMC1)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x00A30000)
      Name (_CCA, One)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x00A30000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 109 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bs1000-gmac" },
          Package () { "max-speed", 1000 },
          Package () { "reg", One },
          Package () { "phy-mode", "rgmii-rxid" },
        }
      })

      Device (GPHY)
      {
        Name (_ADR, Zero)
        Method (_STA)
        {
          Return (0xF)
        }
      }
    }

    Device (MXC0)
    {
      Name (_HID, "BKLE0002")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (0xF)
#endif
      }
      Method (_PXM, 0, NotSerialized) {
        Return(0)
      }
    }

    Device (MUX0)
    {
      Name (_HID, "BKLE0003")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (0xF)
#endif
      }
      Name (MUX, Package () { ^MXC0, 0 })
      Name (DEV0, Package () { ^GP02 })
      Name (DEV1, Package () { ^COM2, I205, I206 })
      Name (STA0, Zero)
      Name (STA1, Zero)
      Method (INIT, 1, Serialized)
      {
        If (LEqual (Arg0, Zero))
        {
          Store (0xF, STA0)
        }
        Else
        {
          Store (0xF, STA1)
        }
      }
    }

    Device (MUX1)
    {
      Name (_HID, "BKLE0003")
      Name (_UID, One)
      Name (_CCA, Zero)
      Method (_STA)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (0xF)
#endif
      }
      Name (MUX, Package () { ^MXC0, 1 })
      Name (DEV0, Package () { ^GP03 })
      Name (DEV1, Package () { ^QSP1 })
      Name (STA0, Zero)
      Name (STA1, Zero)
      Method (INIT, 1, Serialized)
      {
        If (LEqual (Arg0, Zero))
        {
          Store (0xF, STA0)
        }
        Else
        {
          Store (0xF, STA1)
        }
      }
    }

    Device (MUX2)
    {
      Name (_HID, "BKLE0003")
      Name (_UID, 2)
      Name (_CCA, Zero)
      Method (_STA)
      {
#ifdef BAIKAL_MBS_2S
        Return (Zero)
#else
        Return (0xF)
#endif
      }
      Name (MUX, Package () { ^MXC0, 2 })
      Name (DEV0, Package () { ^GP01 })
      Name (DEV1, Package () { ^ESP0 })
      Name (STA0, Zero)
      Name (STA1, Zero)
      Method (INIT, 1, Serialized)
      {
        If (LEqual (Arg0, Zero))
        {
          Store (0xF, STA0)
        }
        Else
        {
          Store (0xF, STA1)
        }
      }
    }

#if (PLATFORM_CHIP_COUNT > 1)
    Include("Dsdt-S1.asi")
#endif
  }
}
