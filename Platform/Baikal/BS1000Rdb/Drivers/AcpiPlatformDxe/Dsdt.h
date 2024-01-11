/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifdef ENABLE_CORESIGHT

#define BAIKAL_DSDT_CORESIGHT_NODES(ChipId)                            \
  Device (ETR##ChipId)                                                 \
  {                                                                    \
    Name (_HID, "ARMHC97C")                                            \
    Name (_UID, 3 * ChipId)                                            \
    Name (_CCA, Zero)                                                  \
    Method (_STA)                                                      \
    {                                                                  \
      Return (0xF)                                                     \
    }                                                                  \
    Method (_CRS, 0, Serialized)                                       \
    {                                                                  \
      Local0 = ResourceTemplate ()                                     \
      {                                                                \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)  \
      }                                                                \
      QWORDBUFSET(01,                                                  \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x02080000),                    \
        0x0000000000000000,                                            \
        0x0000000000001000)                                            \
      Return (Local0)                                                  \
    }                                                                  \
    Name (_DSD, Package ()                                             \
    {                                                                  \
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                 \
      Package ()                                                       \
      {                                                                \
        0,                                                             \
        1,                                                             \
        Package ()                                                     \
        {                                                              \
          1,                                                           \
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),             \
          1,                                                           \
          Package () { 0, 0, \_SB.ETF##ChipId, 0 }                     \
        }                                                              \
      }                                                                \
    })                                                                 \
  }                                                                    \
                                                                       \
  Device (ETF##ChipId)                                                 \
  {                                                                    \
    Name (_HID, "ARMHC97C")                                            \
    Name (_UID, 3 * ChipId + 1)                                        \
    Name (_CCA, Zero)                                                  \
    Method (_STA)                                                      \
    {                                                                  \
      Return (0xF)                                                     \
    }                                                                  \
    Method (_CRS, 0, Serialized)                                       \
    {                                                                  \
      Local0 = ResourceTemplate ()                                     \
      {                                                                \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)  \
      }                                                                \
      QWORDBUFSET(01,                                                  \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x02070000),                    \
        0x0000000000000000,                                            \
        0x0000000000001000)                                            \
      Return (Local0)                                                  \
    }                                                                  \
    Name (_DSD, Package ()                                             \
    {                                                                  \
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                 \
      Package ()                                                       \
      {                                                                \
        0,                                                             \
        1,                                                             \
        Package ()                                                     \
        {                                                              \
          1,                                                           \
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),             \
          2,                                                           \
          Package () { 0, 0, \_SB.STM##ChipId, 0 },                    \
          Package () { 0, 0, \_SB.ETR##ChipId, 1 }                     \
        }                                                              \
      }                                                                \
    })                                                                 \
  }                                                                    \
                                                                       \
  Device (STM##ChipId)                                                 \
  {                                                                    \
    Name (_HID, "ARMHC502")                                            \
    Name (_UID, ChipId)                                                \
    Name (_CCA, Zero)                                                  \
    Method (_STA)                                                      \
    {                                                                  \
      Return (0xF)                                                     \
    }                                                                  \
    Method (_CRS, 0, Serialized)                                       \
    {                                                                  \
      Local0 = ResourceTemplate ()                                     \
      {                                                                \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)  \
        QWORDMEMORYBUF(02, ResourceProducer, NonCacheable, ReadWrite)  \
      }                                                                \
      QWORDBUFSET(01,                                                  \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x02060000),                    \
        0x0000000000000000,                                            \
        0x0000000000001000)                                            \
      QWORDBUFSET(02,                                                  \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x03000000),                    \
        0x0000000000000000,                                            \
        0x0000000001000000)                                            \
      Return (Local0)                                                  \
    }                                                                  \
    Name (_DSD, Package ()                                             \
    {                                                                  \
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                 \
      Package ()                                                       \
      {                                                                \
        0,                                                             \
        1,                                                             \
        Package ()                                                     \
        {                                                              \
          1,                                                           \
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),             \
          1,                                                           \
          Package () { 0, 0, \_SB.ETF##ChipId, 1 }                     \
        }                                                              \
      }                                                                \
    })                                                                 \
  }                                                                    \
                                                                       \
  Device (FUN##ChipId)                                                 \
  {                                                                    \
    Name (_HID, "ARMHC9FF")                                            \
    Name (_UID, ChipId)                                                \
    Name (_CCA, Zero)                                                  \
    Method (_STA)                                                      \
    {                                                                  \
      Return (0xF)                                                     \
    }                                                                  \
    Method (_CRS, 0, Serialized)                                       \
    {                                                                  \
      Local0 = ResourceTemplate ()                                     \
      {                                                                \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)  \
      }                                                                \
      QWORDBUFSET(01,                                                  \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x02050000),                    \
        0x0000000000000000,                                            \
        0x0000000000001000)                                            \
      Return (Local0)                                                  \
    }                                                                  \
    Name (_DSD, Package ()                                             \
    {                                                                  \
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                 \
      Package ()                                                       \
      {                                                                \
        0,                                                             \
        1,                                                             \
        Package ()                                                     \
        {                                                              \
          1,                                                           \
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),             \
          1,                                                           \
          Package () { 0, 0, \_SB.ETB##ChipId, 1 }                     \
        }                                                              \
      }                                                                \
    })                                                                 \
  }                                                                    \
                                                                       \
  Device (ETB##ChipId)                                                 \
  {                                                                    \
    Name (_HID, "ARMHC97C")                                            \
    Name (_UID, 3 * ChipId + 2)                                        \
    Name (_CCA, Zero)                                                  \
    Method (_STA)                                                      \
    {                                                                  \
      Return (0xF)                                                     \
    }                                                                  \
    Method (_CRS, 0, Serialized)                                       \
    {                                                                  \
      Local0 = ResourceTemplate ()                                     \
      {                                                                \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)  \
      }                                                                \
      QWORDBUFSET(01,                                                  \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x02040000),                    \
        0x0000000000000000,                                            \
        0x0000000000001000)                                            \
      Return (Local0)                                                  \
    }                                                                  \
    Name (_DSD, Package ()                                             \
    {                                                                  \
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                 \
      Package ()                                                       \
      {                                                                \
        0,                                                             \
        1,                                                             \
        Package ()                                                     \
        {                                                              \
          1,                                                           \
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),             \
          1,                                                           \
          Package () { 0, 0, \_SB.FUN##ChipId, 0 },                    \
        }                                                              \
      }                                                                \
    })                                                                 \
  }

#define BAIKAL_DSDT_CLUSTER_CORESIGHT_NODE(ChipId, ClusterId, BaseAddr)            \
  Device (FUN0)                                                                    \
  {                                                                                \
    Name (_HID, "ARMHC9FF")                                                        \
    Name (_UID, BAIKAL_ACPI_CLUSTER_ID(ChipId, ClusterId))                         \
    Name (_CCA, Zero)                                                              \
    Method (_STA)                                                                  \
    {                                                                              \
      Return (0xF)                                                                 \
    }                                                                              \
    Method (_CRS, 0, Serialized)                                                   \
    {                                                                              \
      Local0 = ResourceTemplate ()                                                 \
      {                                                                            \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)              \
      }                                                                            \
      QWORDBUFSET(01, BaseAddr + 0x20000, 0x0000000000000000, 0x0000000000001000)  \
      Return (Local0)                                                              \
    }                                                                              \
    Name (_DSD, Package ()                                                         \
    {                                                                              \
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                             \
      Package ()                                                                   \
      {                                                                            \
        0,                                                                         \
        1,                                                                         \
        Package ()                                                                 \
        {                                                                          \
          1,                                                                       \
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),                         \
          5,                                                                       \
          Package () { 0, 0, ^CPU0.ETM0, 0 },                                      \
          Package () { 1, 0, ^CPU1.ETM0, 0 },                                      \
          Package () { 2, 0, ^CPU2.ETM0, 0 },                                      \
          Package () { 3, 0, ^CPU3.ETM0, 0 },                                      \
          Package () { 0, 0, ^ETB0, 1 }                                            \
        }                                                                          \
      }                                                                            \
    })                                                                             \
  }                                                                                \
                                                                                   \
  Device (ETB0)                                                                    \
  {                                                                                \
    Name (_HID, "ARMHC97C")                                                        \
    Name (_UID, BAIKAL_ACPI_CLUSTER_ID(ChipId, ClusterId))                         \
    Name (_CCA, Zero)                                                              \
    Method (_STA)                                                                  \
    {                                                                              \
      Return (0xF)                                                                 \
    }                                                                              \
    Method (_CRS, 0, Serialized)                                                   \
    {                                                                              \
      Local0 = ResourceTemplate ()                                                 \
      {                                                                            \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)              \
      }                                                                            \
      QWORDBUFSET(01, BaseAddr + 0x30000, 0x0000000000000000, 0x0000000000001000)  \
      Return (Local0)                                                              \
    }                                                                              \
    Name (_DSD, Package ()                                                         \
    {                                                                              \
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                             \
      Package ()                                                                   \
      {                                                                            \
        0,                                                                         \
        1,                                                                         \
        Package ()                                                                 \
        {                                                                          \
          1,                                                                       \
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),                         \
          1,                                                                       \
          Package () { 0, 0, ^FUN0, 0 },                                           \
        }                                                                          \
      }                                                                            \
    })                                                                             \
  }

#define BAIKAL_DSDT_CPU_CORESIGHT_NODE(Id, CpuId, BaseAddr)                        \
  Device (DBG0)                                                                    \
  {                                                                                \
    Name (_HID, "ARMHC503")                                                        \
    Name (_UID, CpuId)                                                             \
    Name (_CCA, Zero)                                                              \
    Method (_STA)                                                                  \
    {                                                                              \
      Return (0xF)                                                                 \
    }                                                                              \
    Method (_CRS, 0, Serialized)                                                   \
    {                                                                              \
      Local0 = ResourceTemplate ()                                                 \
      {                                                                            \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)              \
      }                                                                            \
      QWORDBUFSET(01, BaseAddr + 0x10000, 0x0000000000000000, 0x0000000000001000)  \
      Return (Local0)                                                              \
    }                                                                              \
  }                                                                                \
                                                                                   \
  Device (ETM0)                                                                    \
  {                                                                                \
    Name (_HID, "ARMHC500")                                                        \
    Name (_UID, CpuId)                                                             \
    Name (_CCA, Zero)                                                              \
    Method (_STA)                                                                  \
    {                                                                              \
      Return (0xF)                                                                 \
    }                                                                              \
    Method (_CRS, 0, Serialized)                                                   \
    {                                                                              \
      Local0 = ResourceTemplate ()                                                 \
      {                                                                            \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)              \
      }                                                                            \
      QWORDBUFSET(01, BaseAddr + 0x40000, 0x0000000000000000, 0x0000000000001000)  \
      Return (Local0)                                                              \
    }                                                                              \
    Name (_DSD, Package ()                                                         \
    {                                                                              \
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                             \
      Package ()                                                                   \
      {                                                                            \
        0,                                                                         \
        1,                                                                         \
        Package ()                                                                 \
        {                                                                          \
          1,                                                                       \
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),                         \
          1,                                                                       \
          Package () { 0, Id, ^^FUN0, 1 }                                          \
        }                                                                          \
      }                                                                            \
    })                                                                             \
  }

#else

#define BAIKAL_DSDT_CORESIGHT_NODES(ChipId)
#define BAIKAL_DSDT_CLUSTER_CORESIGHT_NODE(ChipId, ClusterId, BaseAddr)
#define BAIKAL_DSDT_CPU_CORESIGHT_NODE(Id, CpuId, BaseAddr)

#endif // ENABLE_CORESIGHT

#define BAIKAL_DSDT_CPU_NODE(Id, CpuId, BaseAddr)        \
  Device (CPU##Id)                                       \
  {                                                      \
    Name (_HID, "ACPI0007")                              \
    Name (_UID, CpuId)                                   \
    Method (_STA)                                        \
    {                                                    \
      Return (0xF)                                       \
    }                                                    \
                                                         \
    BAIKAL_DSDT_CPU_CORESIGHT_NODE(Id, CpuId, BaseAddr)  \
  }

#define BAIKAL_DSDT_CLUSTER_NODE(ChipId, Id, ClusterId, BaseAddr)                    \
  Device (CL##Id)                                                                    \
  {                                                                                  \
    Name (_HID, "ACPI0010")                                                          \
    Name (_UID, BAIKAL_ACPI_CLUSTER_ID(ChipId, ClusterId))                           \
    Method (_STA)                                                                    \
    {                                                                                \
      Return (0xF)                                                                   \
    }                                                                                \
    BAIKAL_DSDT_CPU_NODE (0, 48 * ChipId + 4 * ClusterId, BaseAddr + 0x1000000)      \
    BAIKAL_DSDT_CPU_NODE (1, 48 * ChipId + 4 * ClusterId + 1, BaseAddr + 0x1100000)  \
    BAIKAL_DSDT_CPU_NODE (2, 48 * ChipId + 4 * ClusterId + 2, BaseAddr + 0x1200000)  \
    BAIKAL_DSDT_CPU_NODE (3, 48 * ChipId + 4 * ClusterId + 3, BaseAddr + 0x1300000)  \
                                                                                     \
    BAIKAL_DSDT_CLUSTER_CORESIGHT_NODE(ChipId, ClusterId, BaseAddr)                  \
  }

#define BAIKAL_DSDT_PVT_NODE(ChipId, Id, PvtId, Addr, Irq)                                    \
  Device (P##ChipId##Id)                                                                      \
  {                                                                                           \
    Name (_HID, "PRP0001")                                                                    \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, Addr))                                         \
    Name (_CCA, Zero)                                                                         \
    Method (_CRS, 0, Serialized)                                                              \
    {                                                                                         \
      Local0 = ResourceTemplate ()                                                            \
      {                                                                                       \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                         \
        INTERRUPTBUF_(P##ChipId##Id##_INTERRUPT_ENABLE, 02, Level, ActiveHigh)                \
      }                                                                                       \
      QWORDBUFSET(01,                                                                         \
        PLATFORM_ADDR_OUT_CHIP(ChipId, Addr),                                                 \
        0x0000000000000000,                                                                   \
        0x0000000000001000)                                                                   \
      INTERRUPTSET_(P##ChipId##Id##_INTERRUPT_ENABLE, 02, BAIKAL_SPI_OFFSET_S##ChipId + Irq)  \
      Return (Local0)                                                                         \
    }                                                                                         \
    Name (_DSD, Package ()                                                                    \
    {                                                                                         \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                        \
      Package ()                                                                              \
      {                                                                                       \
        Package () { "compatible", "baikal,bs1000-pvt" },                                     \
        Package () { "baikal,pvt-id", 23 * ChipId + PvtId }                                   \
      }                                                                                       \
    })                                                                                        \
  }

#define BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, Id, PvtId, Addr)            \
  Device (P##ChipId##Id)                                               \
  {                                                                    \
    Name (_HID, "PRP0001")                                             \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, Addr))                  \
    Name (_CCA, Zero)                                                  \
    Method (_CRS, 0, Serialized)                                       \
    {                                                                  \
      Local0 = ResourceTemplate ()                                     \
      {                                                                \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)  \
      }                                                                \
      QWORDBUFSET(01,                                                  \
        PLATFORM_ADDR_OUT_CHIP(ChipId, Addr),                          \
        0x0000000000000000,                                            \
        0x0000000000001000)                                            \
      Return (Local0)                                                  \
    }                                                                  \
    Name (_DSD, Package ()                                             \
    {                                                                  \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                 \
      Package ()                                                       \
      {                                                                \
        Package () { "compatible", "baikal,bs1000-pvt" },              \
        Package () { "baikal,pvt-id", 23 * ChipId + PvtId }            \
      }                                                                \
    })                                                                 \
  }

#define BUS_RES  WordBusNumber (ResourceProducer, MinFixed, MaxFixed,, 0x0, 0x1, 0xFE, 0x0, 0xFE)

#define BAIKAL_DSDT_PCIE_NODES(ChipId, Id, PcieId, PortId, Irq1, Irq2, Irq3)                            \
  Device (PC##ChipId##Id)                                                                               \
  {                                                                                                     \
    Name (_HID, EISAID ("PNP0A08"))                                                                     \
    Name (_CID, EISAID ("PNP0A03"))                                                                     \
    Name (_UID, BAIKAL_ACPI_CHIP_PCIE_SEGMENT(ChipId, BAIKAL_ACPI_PCIE##PcieId##_P##PortId##_SEGMENT))  \
    Name (_CCA, Zero)                                                                                   \
    Name (_SEG, BAIKAL_ACPI_CHIP_PCIE_SEGMENT(ChipId, BAIKAL_ACPI_PCIE##PcieId##_P##PortId##_SEGMENT))  \
    Name (_BBN, Zero)                                                                                   \
                                                                                                        \
    Name (_PRT, Package()                                                                               \
    {                                                                                                   \
      Package() { 0x0000FFFF, 0, Zero, Ones },                                                          \
      Package() { 0x0000FFFF, 1, Zero, Ones },                                                          \
      Package() { 0x0000FFFF, 2, Zero, Ones },                                                          \
      Package() { 0x0000FFFF, 3, Zero, Ones }                                                           \
    })                                                                                                  \
                                                                                                        \
    Method (_CRS, 0, Serialized)                                                                        \
    {                                                                                                   \
      Local0 = ResourceTemplate ()                                                                      \
      {                                                                                                 \
        BUS_RES                                                                                         \
        QWORDMEMORYBUF(01, ResourceProducer,,)                                                          \
        QWORDIOBUF(02)                                                                                  \
      }                                                                                                 \
                                                                                                        \
      QWORDBUFSET(01,                                                                                   \
        BAIKAL_ACPI_PCIE_MEM_BASE,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, BAIKAL_ACPI_PCIE##PcieId##_P##PortId##_MEM_OFFSET),              \
        BAIKAL_ACPI_PCIE_MEM_SIZE)                                                                      \
      QWORDBUFSET(02,                                                                                   \
        BAIKAL_ACPI_PCIE_IO_BASE,                                                 \
        PLATFORM_ADDR_OUT_CHIP(ChipId, BAIKAL_ACPI_PCIE##PcieId##_P##PortId##_IO_OFFSET),               \
        BAIKAL_ACPI_PCIE_IO_SIZE)                                                                       \
                                                                                                        \
      Return (Local0)                                                                                   \
    }                                                                                                   \
                                                                                                        \
    Name (NUMV, 4)                                                                                      \
                                                                                                        \
    Device (RES0)                                                                                       \
    {                                                                                                   \
      Name (_ADR, Zero)                                                                                 \
      Method (_CRS, 0, Serialized)                                                                      \
      {                                                                                                 \
        Local0 = ResourceTemplate ()                                                                    \
        {                                                                                               \
          QWORDMEMORYBUF(01,,,)                                                                         \
          QWORDMEMORYBUF(02, ResourceProducer, NonCacheable, ReadWrite)                                 \
          QWORDMEMORYBUF(03, ResourceProducer, NonCacheable, ReadWrite)                                 \
          QWORDMEMORYBUF(04, ResourceProducer, NonCacheable, ReadWrite)                                 \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 05, Level, ActiveHigh)                       \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 06, Level, ActiveHigh)                       \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 07, Level, ActiveHigh)                       \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 08, Level, ActiveHigh)                       \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 09, Level, ActiveHigh)                       \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 10, Level, ActiveHigh)                       \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 11, Level, ActiveHigh)                       \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 12, Level, ActiveHigh)                       \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 13, Level, ActiveHigh)                       \
          INTERRUPTBUF_(PC##ChipId##Id##_INTERRUPT_ENABLE, 14, Level, ActiveHigh)                       \
        }                                                                                               \
                                                                                                        \
        QWORDBUFSET(01,                                                                                 \
          PLATFORM_ADDR_OUT_CHIP(ChipId, BAIKAL_ACPI_PCIE##PcieId##_P##PortId##_CFG_BASE),              \
          BAIKAL_ACPI_PCIE_CFG_OFFSET,                                                                  \
          BAIKAL_ACPI_PCIE_CFG_SIZE)                                                                    \
        QWORDBUFSET(02,                                                                                 \
          PLATFORM_ADDR_OUT_CHIP(ChipId, BS1000_PCIE##PcieId##_P##PortId##_DBI_BASE),                   \
          0x0000000000000000,                                                                           \
          BS1000_PCIE##PcieId##_P##PortId##_DBI_SIZE)                                                   \
        QWORDBUFSET(03,                                                                                 \
          PLATFORM_ADDR_OUT_CHIP(ChipId, BS1000_PCIE##PcieId##_P##PortId##_ATU_BASE),                   \
          0x0000000000000000,                                                                           \
          BS1000_PCIE##PcieId##_P##PortId##_ATU_SIZE)                                                   \
        QWORDBUFSET(04,                                                                                 \
          PLATFORM_ADDR_OUT_CHIP(ChipId, BAIKAL_ACPI_PCIE##PcieId##_P##PortId##_APB_BASE),              \
          0x0000000000000000,                                                                           \
          BAIKAL_ACPI_PCIE_APB_SIZE)                                                                    \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 05, BAIKAL_SPI_OFFSET_S##ChipId + Irq1)        \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 06, BAIKAL_SPI_OFFSET_S##ChipId + Irq1 + 1)    \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 07, BAIKAL_SPI_OFFSET_S##ChipId + Irq1 + 2)    \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 08, BAIKAL_SPI_OFFSET_S##ChipId + Irq1 + 3)    \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 09, BAIKAL_SPI_OFFSET_S##ChipId + Irq1 + 4)    \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 10, BAIKAL_SPI_OFFSET_S##ChipId + Irq1 + 5)    \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 11, BAIKAL_SPI_OFFSET_S##ChipId + Irq1 + 6)    \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 12, BAIKAL_SPI_OFFSET_S##ChipId + Irq1 + 7)    \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 13, BAIKAL_SPI_OFFSET_S##ChipId + Irq2)        \
        INTERRUPTSET_(PC##ChipId##Id##_INTERRUPT_ENABLE, 14, BAIKAL_SPI_OFFSET_S##ChipId + Irq3)        \
                                                                                                        \
        Return (Local0)                                                                                 \
      }                                                                                                 \
    }                                                                                                   \
                                                                                                        \
    NATIVE_PCIE_OSC                                                                                     \
                                                                                                        \
    Method (_PXM, 0, NotSerialized) {                                                                   \
      Return(ChipId)                                                                                    \
    }                                                                                                   \
  }

#define BAIKAL_DSDT_CHIP_NODES(ChipId)                                                     \
Scope (\_SB)                                                                               \
{                                                                                          \
  BAIKAL_DSDT_CORESIGHT_NODES(ChipId)                                                      \
                                                                                           \
  Device (PKG##ChipId)                                                                     \
  {                                                                                        \
    Name (_HID, "ACPI0010")                                                                \
    Name (_UID, (1 << 16) + ChipId)                                                        \
    Method (_STA)                                                                          \
    {                                                                                      \
      Return (0xF)                                                                         \
    }                                                                                      \
                                                                                           \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 00, 0, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x06000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 01, 1, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x0a000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 02, 2, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x0e000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 03, 3, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x12000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 04, 4, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x16000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 05, 5, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x1a000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 06, 6, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x1e000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 07, 7, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x22000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 08, 8, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x26000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 09, 9, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x2a000000))   \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 0A, 10, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x2e000000))  \
    BAIKAL_DSDT_CLUSTER_NODE (ChipId, 0B, 11, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x32000000))  \
  }                                                                                        \
                                                                                           \
  Device (CK##ChipId##0)                                                                   \
  {                                                                                        \
    Name (_HID, "BKLE1001")                                                                \
    Name (_UID, 2 * ChipId)                                                                \
    Name (_CCA, Zero)                                                                      \
    Method (_STA)                                                                          \
    {                                                                                      \
      Return (0xF)                                                                         \
    }                                                                                      \
    /* CMU addr, clock name */                                                             \
    Name (PROP, Package ()                                                                 \
    {                                                                                      \
      PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00410000),                                          \
      Concatenate(Concatenate("socket", #ChipId), "_sc_cmu1")                              \
    })                                                                                     \
    /* Device reference, clock name, clock id, con_id */                                   \
    Name (CLKS, Package ()                                                                 \
    {                                                                                      \
      ^GM##ChipId##0, "gmac1_apb", 9, "stmmaceth",                                         \
      ^GM##ChipId##0, "gmac1_axi", 10, "axi_clk",                                          \
      ^GM##ChipId##1, "gmac2_apb", 11, "stmmaceth",                                        \
      ^GM##ChipId##1, "gmac2_axi", 12, "axi_clk"                                           \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  Device (CK##ChipId##1)                                                                   \
  {                                                                                        \
    Name (_HID, "BKLE1001")                                                                \
    Name (_UID, 2 * ChipId + 1)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_STA)                                                                          \
    {                                                                                      \
      Return (0xF)                                                                         \
    }                                                                                      \
    /* CMU addr, clock name */                                                             \
    Name (PROP, Package ()                                                                 \
    {                                                                                      \
      PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00420000),                                          \
      Concatenate(Concatenate("socket", #ChipId), "_sc_cmu2")                              \
    })                                                                                     \
    /* Device reference, clock name, clock id, con_id */                                   \
    Name (CLKS, Package ()                                                                 \
    {                                                                                      \
      ^GM##ChipId##0, "gmac1_ptp", 1, "ptp_ref",                                           \
      ^GM##ChipId##0, "gmac1_txx2", 2, "tx2_clk",                                          \
      ^GM##ChipId##1, "gmac2_ptp", 3, "ptp_ref",                                           \
      ^GM##ChipId##1, "gmac2_txx2", 4, "tx2_clk"                                           \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* Cluster PVT */                                                                        \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 00, 0, 0x04030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 01, 1, 0x08030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 02, 2, 0x0C030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 03, 3, 0x10030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 04, 4, 0x14030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 05, 5, 0x18030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 06, 6, 0x1C030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 07, 7, 0x20030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 08, 8, 0x24030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 09, 9, 0x28030000)                                    \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 0A, 10, 0x2C030000)                                   \
  BAIKAL_DSDT_PVT_NOINT_NODE(ChipId, 0B, 11, 0x30030000)                                   \
                                                                                           \
  /* PCIe PVT */                                                                           \
  BAIKAL_DSDT_PVT_NODE(ChipId, 0C, 12, 0x38030000, 195)                                    \
  BAIKAL_DSDT_PVT_NODE(ChipId, 0D, 13, 0x3C030000, 238)                                    \
  BAIKAL_DSDT_PVT_NODE(ChipId, 0E, 14, 0x44030000, 152)                                    \
  BAIKAL_DSDT_PVT_NODE(ChipId, 0F, 15, 0x48030000, 307)                                    \
  BAIKAL_DSDT_PVT_NODE(ChipId, 10, 16, 0x4C030000, 376)                                    \
                                                                                           \
  /* DDR PVT */                                                                            \
  BAIKAL_DSDT_PVT_NODE(ChipId, 11, 17, 0x50030000, 395)                                    \
  BAIKAL_DSDT_PVT_NODE(ChipId, 12, 18, 0x54030000, 412)                                    \
  BAIKAL_DSDT_PVT_NODE(ChipId, 13, 19, 0x58030000, 429)                                    \
  BAIKAL_DSDT_PVT_NODE(ChipId, 14, 20, 0x60030000, 446)                                    \
  BAIKAL_DSDT_PVT_NODE(ChipId, 15, 21, 0x64030000, 463)                                    \
  BAIKAL_DSDT_PVT_NODE(ChipId, 16, 22, 0x68030000, 480)                                    \
                                                                                           \
  /* USB OHCI */                                                                           \
  Device (UB##ChipId##0)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00A00000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00A00000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 110)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "generic-ohci" }                                        \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* USB EHCI */                                                                           \
  Device (UB##ChipId##1)                                                                   \
  {                                                                                        \
    Name (_HID, "PNP0D20")                                                                 \
    Name (_UID, ChipId)                                                                    \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00A10000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 114)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* QSPI_1 */                                                                             \
  Device (SP##ChipId##0)                                                                   \
  {                                                                                        \
    Name (_HID, "HISI0173")                                                                \
    Name (_UID, 2 * ChipId)                                                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C20000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 95)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "num-cs", 4 }                                                         \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* QSPI_2 */                                                                             \
  Device (SP##ChipId##1)                                                                   \
  {                                                                                        \
    Name (_HID, "HISI0173")                                                                \
    Name (_UID, 2 * ChipId + 1)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C30000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 96)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "num-cs", 4 }                                                         \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* ESPI */                                                                               \
  Device (ESP##ChipId)                                                                     \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C40000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C40000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 102)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,bs1000-espi" }                                  \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* GPIO32 */                                                                             \
  Device (GP##ChipId##0)                                                                   \
  {                                                                                        \
    Name (_HID, "APMC0D07")                                                                \
    Name (_UID, 4 * ChipId)                                                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C50000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 86)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
                                                                                           \
    /* GPIO port */                                                                        \
    Device (GPIP)                                                                          \
    {                                                                                      \
      Name (_ADR, Zero)                                                                    \
      Name (_DSD, Package ()                                                               \
      {                                                                                    \
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                   \
        Package ()                                                                         \
        {                                                                                  \
          Package () { "reg", Zero },                                                      \
          Package () { "snps,nr-gpios", 32 }                                               \
        }                                                                                  \
      })                                                                                   \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* GPIO16 */                                                                             \
  Device (GP##ChipId##1)                                                                   \
  {                                                                                        \
    Name (_HID, "APMC0D07")                                                                \
    Name (_UID, 4 * ChipId + 1)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C60000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 83)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
                                                                                           \
    /* GPIO port */                                                                        \
    Device (GPIP)                                                                          \
    {                                                                                      \
      Name (_ADR, Zero)                                                                    \
      Name (_DSD, Package ()                                                               \
      {                                                                                    \
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                   \
        Package ()                                                                         \
        {                                                                                  \
          Package () { "reg", Zero },                                                      \
          Package () { "snps,nr-gpios", 16 }                                               \
        }                                                                                  \
      })                                                                                   \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* GPIO8_1  */                                                                           \
  Device (GP##ChipId##2)                                                                   \
  {                                                                                        \
    Name (_HID, "APMC0D07")                                                                \
    Name (_UID, 4 * ChipId + 2)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C70000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 84)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
                                                                                           \
    /* GPIO port */                                                                        \
    Device (GPIP)                                                                          \
    {                                                                                      \
      Name (_ADR, Zero)                                                                    \
      Name (_DSD, Package ()                                                               \
      {                                                                                    \
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                   \
        Package ()                                                                         \
        {                                                                                  \
          Package () { "reg", Zero },                                                      \
          Package () { "snps,nr-gpios", 8 }                                                \
        }                                                                                  \
      })                                                                                   \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* GPIO8_2 */                                                                            \
  Device (GP##ChipId##3)                                                                   \
  {                                                                                        \
    Name (_HID, "APMC0D07")                                                                \
    Name (_UID, 4 * ChipId + 3)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C80000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 85)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
                                                                                           \
    /* GPIO port */                                                                        \
    Device (GPIP)                                                                          \
    {                                                                                      \
      Name (_ADR, Zero)                                                                    \
      Name (_DSD, Package ()                                                               \
      {                                                                                    \
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                   \
        Package ()                                                                         \
        {                                                                                  \
          Package () { "reg", Zero },                                                      \
          Package () { "snps,nr-gpios", 8 }                                                \
        }                                                                                  \
      })                                                                                   \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* SMBUS_I2C2 */                                                                         \
  Device (I2##ChipId##2)                                                                   \
  {                                                                                        \
    Name (_HID, "APMC0D0F")                                                                \
    Name (_UID, 7 * ChipId + 2)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C90000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 97)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "i2c-sda-hold-time-ns", 500 },                                        \
        Package () { "clock-frequency", 400000 }                                           \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* SMBUS_I2C3 */                                                                         \
  Device (I2##ChipId##3)                                                                   \
  {                                                                                        \
    Name (_HID, "APMC0D0F")                                                                \
    Name (_UID, 7 * ChipId + 3)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CA0000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 98)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "i2c-sda-hold-time-ns", 500 },                                        \
        Package () { "clock-frequency", 400000 }                                           \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* SMBUS_I2C4 */                                                                         \
  Device (I2##ChipId##4)                                                                   \
  {                                                                                        \
    Name (_HID, "APMC0D0F")                                                                \
    Name (_UID, 7 * ChipId + 4)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CB0000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 99)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "i2c-sda-hold-time-ns", 500 },                                        \
        Package () { "clock-frequency", 400000 }                                           \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* SMBUS_I2C5 */                                                                         \
  Device (I2##ChipId##5)                                                                   \
  {                                                                                        \
    Name (_HID, "APMC0D0F")                                                                \
    Name (_UID, 7 * ChipId + 5)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CC0000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 100)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "i2c-sda-hold-time-ns", 500 },                                        \
        Package () { "clock-frequency", 400000 }                                           \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* SMBUS_I2C6 */                                                                         \
  Device (I2##ChipId##6)                                                                   \
  {                                                                                        \
    Name (_HID, "APMC0D0F")                                                                \
    Name (_UID, 7 * ChipId + 6)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CD0000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 101)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "i2c-sda-hold-time-ns", 500 },                                        \
        Package () { "clock-frequency", 400000 }                                           \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* UART_A1 */                                                                            \
  Device (SR##ChipId##0)                                                                   \
  {                                                                                        \
    Name (_HID, "ARMH0011")                                                                \
    Name (_UID, 2 * ChipId)                                                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C00000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 93)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* UART_A2 */                                                                            \
  Device (SR##ChipId##1)                                                                   \
  {                                                                                        \
    Name (_HID, "ARMH0011")                                                                \
    Name (_UID, 2 * ChipId + 1)                                                            \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00C10000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 94)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* UART_S */                                                                             \
  Device (SR##ChipId##2)                                                                   \
  {                                                                                        \
    Name (_HID, "HISI0031")                                                                \
    Name (_UID, ChipId)                                                                    \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00E00000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000000100)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 92)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "reg-shift", 2 },                                                     \
        Package () { "reg-io-width", 4 },                                                  \
        Package () { "clock-frequency", 7372800 }                                          \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* GMAC0 */                                                                              \
  Device (GM##ChipId##0)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00A20000))                                \
    Name (_CCA, One)                                                                       \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00A20000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 108)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,bs1000-gmac" },                                 \
        Package () { "max-speed", 1000 },                                                  \
        Package () { "reg", One },                                                         \
        Package () { "phy-mode", "rgmii-rxid" },                                           \
      }                                                                                    \
    })                                                                                     \
                                                                                           \
    Device (GPHY)                                                                          \
    {                                                                                      \
      Name (_ADR, Zero)                                                                    \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* GMAC1 */                                                                              \
  Device (GM##ChipId##1)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00A30000))                                \
    Name (_CCA, One)                                                                       \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00A30000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 109)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,bs1000-gmac" },                                 \
        Package () { "max-speed", 1000 },                                                  \
        Package () { "reg", One },                                                         \
        Package () { "phy-mode", "rgmii-rxid" },                                           \
      }                                                                                    \
    })                                                                                     \
                                                                                           \
    Device (GPHY)                                                                          \
    {                                                                                      \
      Name (_ADR, Zero)                                                                    \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* MUX */                                                                                \
  Device (MXC##ChipId)                                                                     \
  {                                                                                        \
    Name (_HID, "BKLE0002")                                                                \
    Name (_UID, ChipId)                                                                    \
    Name (_CCA, Zero)                                                                      \
    Method (_PXM, 0, NotSerialized) {                                                      \
      Return(ChipId)                                                                       \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  Device (MX##ChipId##0)                                                                   \
  {                                                                                        \
    Name (_HID, "BKLE0003")                                                                \
    Name (_UID, 3 * ChipId)                                                                \
    Name (_CCA, Zero)                                                                      \
    Name (MUX, Package () { ^MXC##ChipId, 0 })                                             \
    Name (DEV0, Package () { ^GP##ChipId##2 })                                             \
    Name (DEV1, Package () { ^SR##ChipId##2, I2##ChipId##5, I2##ChipId##6 })               \
    Name (STA0, Zero)                                                                      \
    Name (STA1, Zero)                                                                      \
    Method (INIT, 1, Serialized)                                                           \
    {                                                                                      \
      If (LEqual (Arg0, Zero))                                                             \
      {                                                                                    \
        Store (0xF, STA0)                                                                  \
      }                                                                                    \
      Else                                                                                 \
      {                                                                                    \
        Store (0xF, STA1)                                                                  \
      }                                                                                    \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  Device (MX##ChipId##1)                                                                   \
  {                                                                                        \
    Name (_HID, "BKLE0003")                                                                \
    Name (_UID, 3 * ChipId + 1)                                                            \
    Name (_CCA, Zero)                                                                      \
    Name (MUX, Package () { ^MXC##ChipId, 1 })                                             \
    Name (DEV0, Package () { ^GP##ChipId##3 })                                             \
    Name (DEV1, Package () { ^SP##ChipId##1 })                                             \
    Name (STA0, Zero)                                                                      \
    Name (STA1, Zero)                                                                      \
    Method (INIT, 1, Serialized)                                                           \
    {                                                                                      \
      If (LEqual (Arg0, Zero))                                                             \
      {                                                                                    \
        Store (0xF, STA0)                                                                  \
      }                                                                                    \
      Else                                                                                 \
      {                                                                                    \
        Store (0xF, STA1)                                                                  \
      }                                                                                    \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  Device (MX##ChipId##2)                                                                   \
  {                                                                                        \
    Name (_HID, "BKLE0003")                                                                \
    Name (_UID, 3 * ChipId + 2)                                                            \
    Name (_CCA, Zero)                                                                      \
    Name (MUX, Package () { ^MXC##ChipId, 2 })                                             \
    Name (DEV0, Package () { ^GP##ChipId##1 })                                             \
    Name (DEV1, Package () { ^ESP##ChipId })                                               \
    Name (STA0, Zero)                                                                      \
    Name (STA1, Zero)                                                                      \
    Method (INIT, 1, Serialized)                                                           \
    {                                                                                      \
      If (LEqual (Arg0, Zero))                                                             \
      {                                                                                    \
        Store (0xF, STA0)                                                                  \
      }                                                                                    \
      Else                                                                                 \
      {                                                                                    \
        Store (0xF, STA1)                                                                  \
      }                                                                                    \
    }                                                                                      \
  }                                                                                        \
                                                                                           \
  /* Timer1 */                                                                             \
  Device (TM##ChipId##1)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CF0000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CF0000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000000014)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 87)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,dw-apb-timer" },                                \
        Package () { "clock-frequency", 50000000 }                                         \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* Timer2 */                                                                             \
  Device (TM##ChipId##2)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CF0014))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CF0014),                                        \
        0x0000000000000000,                                                                \
        0x0000000000000014)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 88)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,dw-apb-timer" },                                \
        Package () { "clock-frequency", 50000000 }                                         \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* Timer3 */                                                                             \
  Device (TM##ChipId##3)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CF0028))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CF0028),                                        \
        0x0000000000000000,                                                                \
        0x0000000000000014)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 89)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,dw-apb-timer" },                                \
        Package () { "clock-frequency", 50000000 }                                         \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* Timer4 */                                                                             \
  Device (TM##ChipId##4)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CF003C))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CF003C),                                        \
        0x0000000000000000,                                                                \
        0x0000000000000014)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 90)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,dw-apb-timer" },                                \
        Package () { "clock-frequency", 50000000 }                                         \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* WDT */                                                                                \
  Device (WDT##ChipId)                                                                     \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CE0000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x00CE0000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000001000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 91)                                   \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "snps,dw-wdt" },                                        \
        Package () { "clock-frequency", 1000000 },                                         \
        Package () { "snps,watchdog-tops", Package () {                                    \
          0x000000FF, 0x000001FF, 0x000003FF, 0x000007FF,                                  \
          0x0000FFFF, 0x0001FFFF, 0x0003FFFF, 0x0007FFFF,                                  \
          0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF,                                  \
          0x00FFFFFF, 0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF                                   \
        }}                                                                                 \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* DDR0 */                                                                               \
  Device (DR##ChipId##0)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x53000000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
        INTERRUPTBUF(03, Level, ActiveHigh)                                                \
        INTERRUPTBUF(04, Level, ActiveHigh)                                                \
        INTERRUPTBUF(05, Level, ActiveHigh)                                                \
        INTERRUPTBUF(06, Level, ActiveHigh)                                                \
        INTERRUPTBUF(07, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x53000000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 390)                                  \
      INTERRUPTSET(03, BAIKAL_SPI_OFFSET_S##ChipId + 388)                                  \
      INTERRUPTSET(04, BAIKAL_SPI_OFFSET_S##ChipId + 387)                                  \
      INTERRUPTSET(05, BAIKAL_SPI_OFFSET_S##ChipId + 384)                                  \
      INTERRUPTSET(06, BAIKAL_SPI_OFFSET_S##ChipId + 386)                                  \
      INTERRUPTSET(07, BAIKAL_SPI_OFFSET_S##ChipId + 385)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,bs1000-edac-mc" }                               \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* DDR1 */                                                                               \
  Device (DR##ChipId##1)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x57000000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
        INTERRUPTBUF(03, Level, ActiveHigh)                                                \
        INTERRUPTBUF(04, Level, ActiveHigh)                                                \
        INTERRUPTBUF(05, Level, ActiveHigh)                                                \
        INTERRUPTBUF(06, Level, ActiveHigh)                                                \
        INTERRUPTBUF(07, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x57000000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 407)                                  \
      INTERRUPTSET(03, BAIKAL_SPI_OFFSET_S##ChipId + 405)                                  \
      INTERRUPTSET(04, BAIKAL_SPI_OFFSET_S##ChipId + 404)                                  \
      INTERRUPTSET(05, BAIKAL_SPI_OFFSET_S##ChipId + 401)                                  \
      INTERRUPTSET(06, BAIKAL_SPI_OFFSET_S##ChipId + 403)                                  \
      INTERRUPTSET(07, BAIKAL_SPI_OFFSET_S##ChipId + 402)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,bs1000-edac-mc" }                               \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* DDR2 */                                                                               \
  Device (DR##ChipId##2)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x5B000000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
        INTERRUPTBUF(03, Level, ActiveHigh)                                                \
        INTERRUPTBUF(04, Level, ActiveHigh)                                                \
        INTERRUPTBUF(05, Level, ActiveHigh)                                                \
        INTERRUPTBUF(06, Level, ActiveHigh)                                                \
        INTERRUPTBUF(07, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x5B000000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 424)                                  \
      INTERRUPTSET(03, BAIKAL_SPI_OFFSET_S##ChipId + 422)                                  \
      INTERRUPTSET(04, BAIKAL_SPI_OFFSET_S##ChipId + 421)                                  \
      INTERRUPTSET(05, BAIKAL_SPI_OFFSET_S##ChipId + 418)                                  \
      INTERRUPTSET(06, BAIKAL_SPI_OFFSET_S##ChipId + 420)                                  \
      INTERRUPTSET(07, BAIKAL_SPI_OFFSET_S##ChipId + 419)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,bs1000-edac-mc" }                               \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* DDR3 */                                                                               \
  Device (DR##ChipId##3)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x63000000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
        INTERRUPTBUF(03, Level, ActiveHigh)                                                \
        INTERRUPTBUF(04, Level, ActiveHigh)                                                \
        INTERRUPTBUF(05, Level, ActiveHigh)                                                \
        INTERRUPTBUF(06, Level, ActiveHigh)                                                \
        INTERRUPTBUF(07, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x63000000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 441)                                  \
      INTERRUPTSET(03, BAIKAL_SPI_OFFSET_S##ChipId + 439)                                  \
      INTERRUPTSET(04, BAIKAL_SPI_OFFSET_S##ChipId + 438)                                  \
      INTERRUPTSET(05, BAIKAL_SPI_OFFSET_S##ChipId + 435)                                  \
      INTERRUPTSET(06, BAIKAL_SPI_OFFSET_S##ChipId + 437)                                  \
      INTERRUPTSET(07, BAIKAL_SPI_OFFSET_S##ChipId + 436)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,bs1000-edac-mc" }                               \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* DDR4 */                                                                               \
  Device (DR##ChipId##4)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x67000000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
        INTERRUPTBUF(03, Level, ActiveHigh)                                                \
        INTERRUPTBUF(04, Level, ActiveHigh)                                                \
        INTERRUPTBUF(05, Level, ActiveHigh)                                                \
        INTERRUPTBUF(06, Level, ActiveHigh)                                                \
        INTERRUPTBUF(07, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x67000000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 458)                                  \
      INTERRUPTSET(03, BAIKAL_SPI_OFFSET_S##ChipId + 456)                                  \
      INTERRUPTSET(04, BAIKAL_SPI_OFFSET_S##ChipId + 455)                                  \
      INTERRUPTSET(05, BAIKAL_SPI_OFFSET_S##ChipId + 452)                                  \
      INTERRUPTSET(06, BAIKAL_SPI_OFFSET_S##ChipId + 454)                                  \
      INTERRUPTSET(07, BAIKAL_SPI_OFFSET_S##ChipId + 453)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,bs1000-edac-mc" }                               \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  /* DDR5 */                                                                               \
  Device (DR##ChipId##5)                                                                   \
  {                                                                                        \
    Name (_HID, "PRP0001")                                                                 \
    Name (_UID, PLATFORM_ADDR_OUT_CHIP(ChipId, 0x6B000000))                                \
    Name (_CCA, Zero)                                                                      \
    Method (_CRS, 0, Serialized)                                                           \
    {                                                                                      \
      Local0 = ResourceTemplate ()                                                         \
      {                                                                                    \
        QWORDMEMORYBUF(01, ResourceProducer, NonCacheable, ReadWrite)                      \
        INTERRUPTBUF(02, Level, ActiveHigh)                                                \
        INTERRUPTBUF(03, Level, ActiveHigh)                                                \
        INTERRUPTBUF(04, Level, ActiveHigh)                                                \
        INTERRUPTBUF(05, Level, ActiveHigh)                                                \
        INTERRUPTBUF(06, Level, ActiveHigh)                                                \
        INTERRUPTBUF(07, Level, ActiveHigh)                                                \
      }                                                                                    \
      QWORDBUFSET(01,                                                                      \
        PLATFORM_ADDR_OUT_CHIP(ChipId, 0x6B000000),                                        \
        0x0000000000000000,                                                                \
        0x0000000000010000)                                                                \
      INTERRUPTSET(02, BAIKAL_SPI_OFFSET_S##ChipId + 475)                                  \
      INTERRUPTSET(03, BAIKAL_SPI_OFFSET_S##ChipId + 473)                                  \
      INTERRUPTSET(04, BAIKAL_SPI_OFFSET_S##ChipId + 472)                                  \
      INTERRUPTSET(05, BAIKAL_SPI_OFFSET_S##ChipId + 469)                                  \
      INTERRUPTSET(06, BAIKAL_SPI_OFFSET_S##ChipId + 471)                                  \
      INTERRUPTSET(07, BAIKAL_SPI_OFFSET_S##ChipId + 470)                                  \
      Return (Local0)                                                                      \
    }                                                                                      \
    Name (_DSD, Package ()                                                                 \
    {                                                                                      \
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                     \
      Package ()                                                                           \
      {                                                                                    \
        Package () { "compatible", "baikal,bs1000-edac-mc" }                               \
      }                                                                                    \
    })                                                                                     \
  }                                                                                        \
                                                                                           \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 0, 0, 0, 161, 177, 179)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 1, 0, 1, 169, 178, 180)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 2, 1, 0, 204, 220, 222)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 3, 1, 1, 212, 221, 223)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 4, 2, 0, 118, 134, 136)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 5, 2, 1, 126, 135, 137)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 6, 3, 0, 249, 281, 285)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 7, 3, 1, 257, 282, 286)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 8, 3, 2, 265, 283, 287)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, 9, 3, 3, 273, 284, 288)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, A, 4, 0, 318, 350, 354)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, B, 4, 1, 326, 351, 355)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, C, 4, 2, 334, 352, 356)                                   \
  BAIKAL_DSDT_PCIE_NODES(ChipId, D, 4, 3, 342, 353, 357)                                   \
}
