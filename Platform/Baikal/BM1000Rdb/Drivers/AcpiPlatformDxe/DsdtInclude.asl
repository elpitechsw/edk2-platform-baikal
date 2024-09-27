/** @file
  Copyright (c) 2020 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiPlatform.h"

#define ACPI_BAIKAL_PWM_LSP_CLK(Channel, Mask)                \
  ACPI_BAIKAL_SMC_CMU_DATA                                    \
  PowerResource (PWRR, 0, 0)                                  \
  {                                                           \
    Name (PWRV, 1)                                            \
    Method (_STA)                                             \
    {                                                         \
      Return (PWRV)                                           \
    }                                                         \
    OperationRegion (RBUF, SystemMemory, 0x20050000, 0x4)     \
    Field (RBUF, DwordAcc, NoLock, Preserve)                  \
    {                                                         \
      RST, 0x20                                               \
    }                                                         \
    Method (_ON)                                              \
    {                                                         \
      ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, Channel)      \
      RST &= ~Mask                                            \
      PWRV = 1                                                \
    }                                                         \
    Method (_OFF)                                             \
    {                                                         \
      RST |= Mask                                             \
      ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, Channel)     \
      PWRV = 0                                                \
    }                                                         \
  }                                                           \
  Name (_PR0, Package () { PWRR })                            \
  Name (_PR3, Package () { PWRR })                            \
  ACPI_BAIKAL_PWM_PS_METHODS

#define BAIKAL_ACPI_PROCESSOR_LPI                         \
  Name (_LPI, Package ()                                  \
  {                                                       \
    0,                                                    \
    0,                                                    \
    2,                                                    \
    Package ()                                            \
    {                                                     \
      1,                                                  \
      1,                                                  \
      1,                                                  \
      0,                                                  \
      0,                                                  \
      0,                                                  \
      ResourceTemplate ()                                 \
      {                                                   \
        Register (FFixedHW, 0x20, 0x00, 0xFFFFFFFF, 0x03) \
      },                                                  \
      ResourceTemplate ()                                 \
      {                                                   \
        Register (SystemMemory, 0, 0, 0, 0)               \
      },                                                  \
      ResourceTemplate ()                                 \
      {                                                   \
        Register (SystemMemory, 0, 0, 0, 0)               \
      },                                                  \
      "ARM WFI"                                           \
    },                                                    \
    Package ()                                            \
    {                                                     \
      2000,                                               \
      1500,                                               \
      1,                                                  \
      1,                                                  \
      0,                                                  \
      1,                                                  \
      ResourceTemplate ()                                 \
      {                                                   \
        Register (FFixedHW, 0x20, 0x00, 0x00000001, 0x03) \
      },                                                  \
      ResourceTemplate ()                                 \
      {                                                   \
        Register (SystemMemory, 0, 0, 0, 0)               \
      },                                                  \
      ResourceTemplate ()                                 \
      {                                                   \
        Register (SystemMemory, 0, 0, 0, 0)               \
      },                                                  \
      "cpu-sleep"                                         \
    }                                                     \
  })

#ifdef ENABLE_CORESIGHT

#define BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(ClusterId, Id, PortId, EtmAddr, DbgAddr)  \
          Device (ETM0)                                                                 \
          {                                                                             \
            Name (_HID, "ARMHC500")                                                     \
            Name (_UID, Id)                                                             \
            Name (_CCA, Zero)                                                           \
            Method (_STA)                                                               \
            {                                                                           \
              Return (0xF)                                                              \
            }                                                                           \
            Name (_CRS, ResourceTemplate ()                                             \
            {                                                                           \
              Memory32Fixed (ReadWrite, EtmAddr, 0x1000)                                \
            })                                                                          \
            Name (_DSD, Package ()                                                      \
            {                                                                           \
              ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                          \
              Package ()                                                                \
              {                                                                         \
                0,                                                                      \
                1,                                                                      \
                Package ()                                                              \
                {                                                                       \
                  1,                                                                    \
                  ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),                      \
                  1,                                                                    \
                  Package () { 0, PortId, \_SB.PKG.CLU ## ClusterId.FUN0, 1 }           \
                }                                                                       \
              }                                                                         \
            })                                                                          \
          }                                                                             \
                                                                                        \
          Device (DBG0)                                                                 \
          {                                                                             \
            Name (_HID, "ARMHC503")                                                     \
            Name (_UID, Id)                                                             \
            Name (_CCA, Zero)                                                           \
            Method (_STA)                                                               \
            {                                                                           \
              Return (0xF)                                                              \
            }                                                                           \
            Name (_CRS, ResourceTemplate ()                                             \
            {                                                                           \
              Memory32Fixed (ReadWrite, DbgAddr, 0x1000)                                \
            })                                                                          \
          }

#define BAIKAL_ACPI_CLUSTER_CORESIGHT_NODES(Id, EtfAddr)              \
        Device (FUN0)                                                 \
        {                                                             \
          Name (_HID, "ARMHC9FE")                                     \
          Name (_UID, Id)                                             \
          Name (_CCA, Zero)                                           \
          Method (_STA)                                               \
          {                                                           \
            Return (0xF)                                              \
          }                                                           \
          Name (_DSD, Package ()                                      \
          {                                                           \
            ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),          \
            Package ()                                                \
            {                                                         \
              0,                                                      \
              1,                                                      \
              Package ()                                              \
              {                                                       \
                1,                                                    \
                ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),      \
                3,                                                    \
                Package () { 0, 0, \_SB.PKG.CLU ## Id.CPU0.ETM0, 0 }, \
                Package () { 1, 0, \_SB.PKG.CLU ## Id.CPU1.ETM0, 0 }, \
                Package () { 0, 0, \_SB.PKG.CLU ## Id.ETF0, 1 }       \
              }                                                       \
            }                                                         \
          })                                                          \
        }                                                             \
                                                                      \
        Device (ETF0)                                                 \
        {                                                             \
          Name (_HID, "ARMHC97C")                                     \
          Name (_UID, Id)                                             \
          Name (_CCA, Zero)                                           \
          Method (_STA)                                               \
          {                                                           \
            Return (0xF)                                              \
          }                                                           \
          Name (_CRS, ResourceTemplate ()                             \
          {                                                           \
            Memory32Fixed (ReadWrite, EtfAddr, 0x1000)                \
          })                                                          \
          Name (_DSD, Package ()                                      \
          {                                                           \
            ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),          \
            Package ()                                                \
            {                                                         \
              0,                                                      \
              1,                                                      \
              Package ()                                              \
              {                                                       \
                1,                                                    \
                ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),      \
                2,                                                    \
                Package () { 0, 0, \_SB.PKG.CLU ## Id.FUN0, 0 },      \
                Package () { 0, Id, \_SB.FUN0, 1 }                    \
              }                                                       \
            }                                                         \
          })                                                          \
        }

#else

#define BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(ClusterId, Id, PortId, EtmAddr, DbgAddr)
#define BAIKAL_ACPI_CLUSTER_CORESIGHT_NODES(Id, EtfAddr)

#endif // ENABLE_CORESIGHT

Name (\_SB.CSTA, 0)
Method (_TTS, 1)
{
  \_SB.CSTA = Arg0
}

Scope (_SB)
{
  PowerResource (SATP, 0, 0)
  {
    Name (PWRV, 1)
    Method (_STA)
    {
      Return (PWRV)
    }
    OperationRegion (RBUF, SystemMemory, 0x2C050000, 0x4)
    Field (RBUF, DwordAcc, NoLock, Preserve)
    {
      RST, 0x20
    }
    ACPI_BAIKAL_SMC_CMU_DATA
    Method (_ON)
    {
      ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 0)
      RST &= ~0x10000
      PWRV = 1
    }
    Method (_OFF)
    {
      RST |= 0x10000
      ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 0)
      PWRV = 0
    }
  }

  PowerResource (XGBP, 0, 0)
  {
    Name (PWRV, 1)
    Method (_STA)
    {
      Return (PWRV)
    }
    ACPI_BAIKAL_SMC_CMU_DATA
    Method (_ON)
    {
      ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 0)
      PWRV = 1
    }
    Method (_OFF)
    {
      ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 0)
      PWRV = 0
    }
  }

  PowerResource (XGMP, 0, 0)
  {
    Name (PWRV, 1)
    Method (_STA)
    {
      Return (PWRV)
    }
    ACPI_BAIKAL_SMC_CMU_DATA
    Method (_ON)
    {
      ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 1)
      PWRV = 1
    }
    Method (_OFF)
    {
      ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 1)
      PWRV = 0
    }
  }

  Method (_OSC, 4)
  {
    CreateDWordField (Arg3, Zero, CDW1)

    /* Check for proper UUID */
    If (LEqual (Arg0, ToUUID ("0811B06E-4A27-44F9-8D60-3CBBC22E7B48")))
    {
      /* Allow everything by default */

      /* Unknown revision */
      If (LNotEqual (Arg1, One))
      {
        Or (CDW1, 0x08, CDW1)
      }
    }
    Else
    {
      /* Unrecognized UUID */
      Or (CDW1, 4, CDW1)
    }

    Return (Arg3)
  }

#ifdef ENABLE_CORESIGHT
  Device (STM0)
  {
    Name (_HID, "ARMHC502")
    Name (_UID, Zero)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x05001000, 0x1000)
      Memory32Fixed (ReadWrite, 0x07000000, 0x1000000)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),
      Package ()
      {
        0,
        1,
        Package ()
        {
          1,
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),
          1,
          Package () { 0, 4, \_SB.FUN0, 1 }
        }
      }
    })
  }

  Device (FUN0)
  {
    Name (_HID, "ARMHC9FF")
    Name (_UID, Zero)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x05011000, 0x1000)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),
      Package ()
      {
        0,
        1,
        Package ()
        {
          1,
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),
          6,
          Package () { 0, 0, \_SB.PKG.CLU0.ETF0, 0 },
          Package () { 1, 0, \_SB.PKG.CLU1.ETF0, 0 },
          Package () { 2, 0, \_SB.PKG.CLU2.ETF0, 0 },
          Package () { 3, 0, \_SB.PKG.CLU3.ETF0, 0 },
          Package () { 4, 0, \_SB.STM0, 0 },
          Package () { 0, 0, \_SB.ETF0, 1 }
        }
      }
    })
  }

  Device (ETF0)
  {
    Name (_HID, "ARMHC97C")
    Name (_UID, 4)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x0500D000, 0x1000)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),
      Package ()
      {
        0,
        1,
        Package ()
        {
          1,
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),
          2,
          Package () { 0, 0, \_SB.FUN0, 0 },
          Package () { 0, 0, \_SB.REP0, 1 }
        }
      }
    })
  }

  Device (REP0)
  {
    Name (_HID, "ARMHC98D")
    Name (_UID, Zero)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x05010000, 0x1000)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),
      Package ()
      {
        0,
        1,
        Package ()
        {
          1,
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),
          3,
          Package () { 0, 0, \_SB.ETF0, 0 },
          Package () { 0, 0, \_SB.ETR0, 1 },
          Package () { 1, 0, \_SB.ETB0, 1 }
        }
      }
    })
  }

  Device (ETR0)
  {
    Name (_HID, "ARMHC97C")
    Name (_UID, 5)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x05008000, 0x1000)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),
      Package ()
      {
        0,
        1,
        Package ()
        {
          1,
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),
          1,
          Package () { 0, 0, \_SB.REP0, 0 }
        }
      }
    })
  }

  Device (ETB0)
  {
    Name (_HID, "ARMHC97C")
    Name (_UID, 6)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x05007000, 0x1000)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),
      Package ()
      {
        0,
        1,
        Package ()
        {
          1,
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),
          2,
          Package () { 0, 1, \_SB.REP0, 0 },
          Package () { 0, 0, \_SB.TPIU, 1 }
        }
      }
    })
  }

  Device (TPIU)
  {
    Name (_HID, "ARMHC979")
    Name (_UID, Zero)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x0500E000, 0x1000)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),
      Package ()
      {
        0,
        1,
        Package ()
        {
          1,
          ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),
          1,
          Package () { 0, 0, \_SB.ETB0, 0 }
        }
      }
    })
  }
#endif // ENABLE_CORESIGHT

  Device (PKG)
  {
    Name (_HID, "ACPI0010")
    Name (_UID, 12)
    Method (_STA)
    {
      Return (0xF)
    }

    // Cluster 0
    Device (CLU0)
    {
      Name (_HID, "ACPI0010")
      Name (_UID, 8)
      Method (_STA)
      {
        Return (0xF)
      }

      // Cpu 0
      Device (CPU0)
      {
        Name (_HID, "ACPI0007")
        Name (_UID, Zero)
        Method (_STA)
        {
          Return (0xF)
        }

        BAIKAL_ACPI_PROCESSOR_LPI

        BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(0, 0, 0, 0x05240000, 0x05210000)
      }

      // Cpu 1
      Device (CPU1)
      {
        Name (_HID, "ACPI0007")
        Name (_UID, One)
        Method (_STA)
        {
          Return (0xF)
        }

        BAIKAL_ACPI_PROCESSOR_LPI

        BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(0, 1, 1, 0x05340000, 0x05310000)
      }

      BAIKAL_ACPI_CLUSTER_CORESIGHT_NODES(0, 0x05009000)
    }

    // Cluster 1
    Device (CLU1)
    {
      Name (_HID, "ACPI0010")
      Name (_UID, 9)
      Method (_STA)
      {
        Return (0xF)
      }

      // Cpu 2
      Device (CPU0)
      {
        Name (_HID, "ACPI0007")
        Name (_UID, 2)
        Method (_STA)
        {
          Return (0xF)
        }

        BAIKAL_ACPI_PROCESSOR_LPI

        BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(1, 2, 0, 0x05440000, 0x05410000)
      }

      // Cpu 3
      Device (CPU1)
      {
        Name (_HID, "ACPI0007")
        Name (_UID, 3)
        Method (_STA)
        {
          Return (0xF)
        }

        BAIKAL_ACPI_PROCESSOR_LPI

        BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(1, 3, 1, 0x05540000, 0x05510000)
      }

      BAIKAL_ACPI_CLUSTER_CORESIGHT_NODES(1, 0x0500A000)
    }

    // Cluster 2
    Device (CLU2)
    {
      Name (_HID, "ACPI0010")
      Name (_UID, 10)
      Method (_STA)
      {
        Return (0xF)
      }

      // Cpu 4
      Device (CPU0)
      {
        Name (_HID, "ACPI0007")
        Name (_UID, 4)
        Method (_STA)
        {
          Return (0xF)
        }

        BAIKAL_ACPI_PROCESSOR_LPI

        BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(2, 4, 0, 0x05640000, 0x05610000)
      }

      // Cpu 5
      Device (CPU1)
      {
        Name (_HID, "ACPI0007")
        Name (_UID, 5)
        Method (_STA)
        {
          Return (0xF)
        }

        BAIKAL_ACPI_PROCESSOR_LPI

        BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(2, 5, 1, 0x05740000, 0x05710000)
      }

      BAIKAL_ACPI_CLUSTER_CORESIGHT_NODES(2, 0x0500B000)
    }

    // Cluster 3
    Device (CLU3)
    {
      Name (_HID, "ACPI0010")
      Name (_UID, 11)
      Method (_STA)
      {
        Return (0xF)
      }

      // Cpu 6
      Device (CPU0)
      {
        Name (_HID, "ACPI0007")
        Name (_UID, 6)
        Method (_STA)
        {
          Return (0xF)
        }

        BAIKAL_ACPI_PROCESSOR_LPI

        BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(3, 6, 0, 0x05840000, 0x05810000)
      }

      // Cpu 7
      Device (CPU1)
      {
        Name (_HID, "ACPI0007")
        Name (_UID, 7)
        Method (_STA)
        {
          Return (0xF)
        }

        BAIKAL_ACPI_PROCESSOR_LPI

        BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(3, 7, 1, 0x05940000, 0x05910000)
      }

      BAIKAL_ACPI_CLUSTER_CORESIGHT_NODES(3, 0x0500C000)
    }
  }

  Device (CLK0)
  {
    Name (_HID, "BKLE0001")
    Name (_UID, Zero)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    /* CMU id, clock name, frequency, is_osc27 */
    Name (PROP, Package ()
    {
      0x28000000, "bm1000-cluster0-cmu0", 1500000000, Zero
    })
    /* Device reference, con_id */
    Name (CMU, Package ()
    {
      ^PKG.CLU0.CPU0, Zero,
      ^PKG.CLU0.CPU1, Zero
    })
  }

  Device (CLK1)
  {
    Name (_HID, "BKLE0001")
    Name (_UID, One)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    /* CMU id, clock name, frequency, is_osc27 */
    Name (PROP, Package ()
    {
      0x0C000000, "bm1000-cluster1-cmu0", 1500000000, Zero
    })
    /* Device reference, con_id */
    Name (CMU, Package ()
    {
      ^PKG.CLU1.CPU0, Zero,
      ^PKG.CLU1.CPU1, Zero
    })
  }

  Device (CLK2)
  {
    Name (_HID, "BKLE0001")
    Name (_UID, 2)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    /* CMU id, clock name, frequency, is_osc27 */
    Name (PROP, Package ()
    {
      0x0A000000, "bm1000-cluster2-cmu0", 1500000000, Zero
    })
    /* Device reference, con_id */
    Name (CMU, Package ()
    {
      ^PKG.CLU2.CPU0, Zero,
      ^PKG.CLU2.CPU1, Zero
    })
  }

  Device (CLK3)
  {
    Name (_HID, "BKLE0001")
    Name (_UID, 3)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    /* CMU id, clock name, frequency, is_osc27 */
    Name (PROP, Package ()
    {
      0x26000000, "bm1000-cluster3-cmu0", 1500000000, Zero
    })
    /* Device reference, con_id */
    Name (CMU, Package ()
    {
      ^PKG.CLU3.CPU0, Zero,
      ^PKG.CLU3.CPU1, Zero
    })
  }

  Device (CLK4)
  {
    Name (_HID, "BKLE0001")
    Name (_UID, 6)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    /* CMU id, clock name, frequency, is_osc27 */
    Name (PROP, Package ()
    {
      0x2A000000, "bm1000-mali-cmu0", 750000000, Zero
    })
    /* Device reference, clock name, clock id, con_id */
    Name (CLKS, Package ()
    {
      ^GPU0, "aclk", 0xFFFFFFFFFFFFFFFF, Zero
    })
  }

  Device (CLK5)
  {
    Name (_HID, "BKLE0001")
    Name (_UID, 7)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    /* CMU id, clock name, frequency, is_osc27 */
    Name (PROP, Package ()
    {
      0x2C000000, "bm1000-usb-cmu0", 800000000, Zero
    })
    /* Device reference, clock name, clock id, con_id */
    Name (CLKS, Package ()
    {
      ^DMA1, "dmac_aclk", 15, "apb_pclk",
    })
  }

  Device (CLK6)
  {
    Name (_HID, "BKLE0001")
    Name (_UID, 8)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    /* CMU id, clock name, frequency, is_osc27 */
    Name (PROP, Package ()
    {
      0x30000000, "bm1000-xgbe-cmu0", 1250000000, Zero
    })
    /* Device reference, clock name, clock id, con_id */
    Name (CLKS, Package ()
    {
      ^XGM0, "104mhz_clk", 1, "pclk",
      ^XGM0, "phy0_core_refclk", 2, "tx",
      ^XGM0, "xgbe0_aclk", 3, "stmmaceth",
      ^XGM0, "xgbe0_ptpclk", 4, "ptp_ref",
      /* ^XMI0, "104mhz_clk", 1, "pclk", */
      /* ^XMI0.XPC0, "phy0_core_refclk", 2, "core", */
      /* ^XGM1, "104mhz_clk", 1, "pclk", */
      ^XGM1, "phy1_core_refclk", 5, "tx",
      ^XGM1, "xgbe1_aclk", 6, "stmmaceth",
      ^XGM1, "xgbe1_ptpclk", 7, "ptp_ref",
      /* ^XMI1, "104mhz_clk", 1, "pclk", */
      /* ^XMI1.XPC1, "phy1_core_refclk", 5, "core", */
      ^GMC0, "gmac0_aclk", 8, "stmmaceth",
      ^GMC0, "gmac0_ptpclk", 9, "ptp_clk",
      ^GMC0, "gmac0_tx2", 10, "tx",
      ^GMC1, "gmac1_aclk", 11, "stmmaceth",
      ^GMC1, "gmac1_ptpclk", 12, "ptp_clk",
      ^GMC1, "gmac1_tx2", 13, "tx",
    })
  }

  Device (TMR1)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20290000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20290000, 0x14)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 127 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-dw-apb-timer" },
        Package () { "clock-frequency", 50000000 }
      }
    })

    ACPI_BAIKAL_PWM_LSP_CLK (8, 0x40020000)
  }

  Device (TMR2)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20290014)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20290014, 0x14)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 128 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-dw-apb-timer" },
        Package () { "clock-frequency", 50000000 }
      }
    })

    ACPI_BAIKAL_PWM_LSP_CLK (9, 0x40040000)
  }

  Device (TMR3)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20290028)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20290028, 0x14)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 129 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-dw-apb-timer" },
        Package () { "clock-frequency", 50000000 }
      }
    })

    ACPI_BAIKAL_PWM_LSP_CLK (10, 0x40080000)
  }

  Device (TMR4)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x2029003C)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x2029003C, 0x14)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 130 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-dw-apb-timer" },
        Package () { "clock-frequency", 50000000 }
      }
    })

    ACPI_BAIKAL_PWM_LSP_CLK (11, 0x40100000)
  }

  // CCN
  Device (CCN0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x09000000)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x09000000, 0x1000000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 159 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "arm,ccn-504" }
      }
    })
  }

  // PCIe GPR
  Device (PGPR)
  {
    Name (_ADR, 0x02050000)
    Name (_UID, 0x02050000)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x02050000, 0x100)
    })
  }

  // PVT2
  Device (PVT2)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x0A200000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x0A200000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 155 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-pvt" }
      }
    })
  }

  // PVT1
  Device (PVT1)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x0C200000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x0C200000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 153 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-pvt" }
      }
    })
  }

  // DDR0
  Device (DDR0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x0E200000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x0E200000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 161, 162, 163, 164, 165, 166 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-edac-mc" }
      }
    })
  }

  // DDR1
  Device (DDR1)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x22200000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x22200000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 171, 172, 173, 174, 175, 176 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-edac-mc" }
      }
    })
  }

  // GPIO
  Device (GPIO)
  {
    Name (_HID, "APMC0D07")
    Name (_UID, Zero)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20200000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 131 }
    })

    // GPIO port
    Device (GPIP)
    {
      Name (_ADR, Zero)
    }
  }

  // SPI
  Device (SPI0)
  {
    Name (_HID, "HISI0173")
    Name (_UID, Zero)
    Name (_CCA, Zero)
    Method (_CRS)
    {
      Local0 = ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x20210000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 132 }
        FixedDMA (8, 8)
        FixedDMA (9, 9)
      }

      If (CondRefOf (\_SB.SPI0.CSGP))
      {
        Local0 = ConcatenateResTemplate (Local0, ResourceTemplate ()
        {
          GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 24, 25, 26, 27 }
        })
      }

      Return (Local0)
    }

#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
    ACPI_BAIKAL_PWM_LSP_CLK (4, 0x04002000)
#endif

    Method (_DSD)
    {
      Local0 = Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "num-cs", 4 },
          Package () { "max-freq", 50000000 },
          Package () { "dma-names", Package () { "tx", "rx" } },
          Package () { "baikal,masters", Package () {
            0, 0, 0, 0, 0, 0, 0, 0,
            ^^DMA0, 1, 0,
            ^^DMA0, 1, 0
          }},
          Package () { "cs-gpios", Package () { Zero } }
        }
      }

      If (CondRefOf (\_SB.SPI0.CSGP, Local1))
      {
        DerefOf (DerefOf (Local0[1])[4])[1] = DerefOf (Local1)
      }

      Return (Local0)
    }
  }

  // DMAC LSP
  Device (DMA0) {
    Name (_HID, "BKLE0005")
    Name (_UID, 0x202B0000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x202B0000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
      {
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
        51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
        61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
        71, 72, 73, 74, 75, 76, 77, 78, 79, 80
      }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "dma-channels", 8 },
        Package () { "dma-requests", 16 },
        Package () { "dma-masters", 2 },
        Package () { "chan_allocation_order", Zero },
        Package () { "chan_priority", Zero },
        Package () { "block_size", 0xFF },
        Package () { "data-width", Package () { 4, 16 } },
        Package () { "multi-block", Package () { One, One, One, One, One, One, One, One } },
        Package () { "snps,max-burst-len", Package () { 32, 32, 32, 32, 32, 32, 32, 32 } },
        Package () { "snps,dma-protection-control", One }
      }
    })
  }

  // DMAC M2M
  Device (DMA1) {
    Name (_HID, "ARMH0330")
    Name (_UID, 0x2C630000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x2C630000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
      {
        255, 256, 257, 258, 259, 260, 261, 262, 263
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 15)
        PWRV = 1
      }
      Method (_OFF)
      {
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 15)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { PWRR })
    Name (_PR3, Package () { PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }

  // UART1
  Device (COM0)
  {
    Name (_HID, "HISI0031")
    Name (_UID, Zero)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20230000, 0x100)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 133 }
      FixedDMA (0, 0)
      FixedDMA (1, 1)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "reg-shift", 2 },
        Package () { "reg-io-width", 4 },
        Package () { "clock-frequency", 7361963 },
        Package () { "dma-names", Package () { "tx", "rx" } },
        Package () { "baikal,masters", Package () {
          ^DMA0, 1, 0,
          ^DMA0, 1, 0
        }}
      }
    })
  }

  // UART2
  Device (COM1)
  {
    Name (_HID, "HISI0031")
    Name (_UID, One)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20240000, 0x100)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 134 }
      FixedDMA (2, 2)
      FixedDMA (3, 3)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "reg-shift", 2 },
        Package () { "reg-io-width", 4 },
        Package () { "clock-frequency", 7361963 },
        Package () { "dma-names", Package () { "tx", "rx" } },
        Package () { "baikal,masters", Package () {
          0, 0,
          ^DMA0, 1, 0,
          ^DMA0, 1, 0
        }}
      }
    })

    ACPI_BAIKAL_PWM_LSP_CLK (2, 0x02001000)
  }

  // I2C1
  Device (I2C1)
  {
    Name (_HID, "APMC0D0F")
    Name (_UID, Zero)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20250000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 140 }
      FixedDMA (4, 4)
      FixedDMA (5, 5)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "i2c-sda-hold-time-ns", 500 },
        Package () { "clock-frequency", 400000 },
        Package () { "dma-names", Package () { "tx", "rx" } },
        Package () { "baikal,masters", Package () {
          0, 0, 0, 0,
          ^DMA0, 1, 0,
          ^DMA0, 1, 0
        }}
      }
    })

#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
    ACPI_BAIKAL_PWM_LSP_CLK (6, 0x10008000)
#endif
  }

  // I2C2
  Device (I2C2)
  {
    Name (_HID, "APMC0D0F")
    Name (_UID, One)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20260000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 141 }
      FixedDMA (6, 6)
      FixedDMA (7, 7)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "i2c-sda-hold-time-ns", 500 },
        Package () { "clock-frequency", 400000 },
        Package () { "dma-names", Package () { "tx", "rx" } },
        Package () { "baikal,masters", Package () {
          0, 0, 0, 0, 0, 0,
          ^DMA0, 1, 0,
          ^DMA0, 1, 0
        }}
      }
    })

    ACPI_BAIKAL_PWM_LSP_CLK (7, 0x20010000)
  }

  // SMBUS1
  Device (SMB0)
  {
    Name (_HID, "PRP0001")
    Name (_CID, "PNP0500")
    Name (_UID, 0x20270000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20270000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 142 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-smbus" },
        Package () { "clock-frequency", 100000 },
        Package () { "smbus-clock-frequency", 50000000 }
      }
    })

    ACPI_BAIKAL_PWM_LSP_CLK (13, 0x00200000)
  }

  // SMBUS2
  Device (SMB1)
  {
    Name (_HID, "PRP0001")
    Name (_CID, "PNP0500")
    Name (_UID, 0x20280000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20280000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 143 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-smbus" },
        Package () { "clock-frequency", 100000 },
        Package () { "smbus-clock-frequency", 50000000 }
      }
    })

    ACPI_BAIKAL_PWM_LSP_CLK (14, 0x00400000)
  }

  // ESPI
  Device (ESPI)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x202A0000)
    Name (_CCA, Zero)
    Method (_CRS)
    {
      Local0 = ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x202A0000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 135 }
      }

      If (CondRefOf (\_SB.ESPI.CSGP))
      {
        Local0 = ConcatenateResTemplate (Local0, ResourceTemplate ()
        {
          GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 0 }
        })
      }

      Return (Local0)
    }
    Method (_DSD)
    {
      Local0 = Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bm1000-espi" },
          Package () { "cs-gpios", Package () { Zero } }
        }
      }

      If (CondRefOf (\_SB.ESPI.CSGP, Local1))
      {
        DerefOf (DerefOf (Local0[1])[1])[1] = DerefOf (Local1)
      }

      Return (Local0)
    }

    ACPI_BAIKAL_PWM_LSP_CLK (5, 0x08004000)

    Method (CSET, 1)
    {
      ACPI_BAIKAL_CMU_CLKCH_SET_RATE (0x20000000, 5, Arg0)
    }

    Method (CGET)
    {
      ACPI_BAIKAL_CMU_CLKCH_GET_RATE (0x20000000, 5)
      Return (ID0)
    }
  }

  // SD/eMMC
  Device (MMC0)
  {
    Name (_HID, "BKLE0004")
    Name (_CID, "PNP0D40")
    Name (_UID, 0x202E0000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x202E0000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 83, 84 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "no-1-8-v", 1 },
        Package () { "no-sdio", 1 },
#ifdef USE_EMMC
        Package () { "max-frequency", 20000000 },
        Package () { "bus-width", 8 },
        Package () { "non-removable", 1 },
        Package () { "no-sd", 1 },
#else
        Package () { "max-frequency", 25000000 },
        Package () { "bus-width", 4 },
        Package () { "disable-wp", 1 },
        Package () { "no-mmc", 1 },
#endif
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      OperationRegion (RBUF, SystemMemory, 0x20050008, 0x4)
      Field (RBUF, DwordAcc, NoLock, Preserve)
      {
        RST, 0x20
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, 17)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, 18)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, 19)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, 20)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, 21)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, 22)
        RST &= ~0x7E
        PWRV = 1
      }
      Method (_OFF)
      {
        RST |= 0x7E
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, 22)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, 21)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, 20)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, 19)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, 18)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, 17)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { PWRR })
    Name (_PR3, Package () { PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }

  // PVT3
  Device (PVT3)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x26200000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x26200000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 157 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-pvt" }
      }
    })
  }

  // PVT0
  Device (PVT0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x28200000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x28200000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 151 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-pvt" }
      }
    })
  }

  // PVT_MALI
  Device (PVTM)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x2A060000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x2A060000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 253 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-pvt" }
      }
    })
  }

  // USB2
  Device (USB2)
  {
    Name (_HID, "808622B7")
    Name (_CID, "PNP0D10")
    Name (_UID, Zero)
    Name (_CCA, One)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x2C400000, 0x100000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 267, 268, 277 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "dr_mode", "host" },
        Package () { "maximum-speed", "high-speed" }
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      OperationRegion (RBUF, SystemMemory, 0x2C050000, 0x4)
      Field (RBUF, DwordAcc, NoLock, Preserve)
      {
        RST, 0x20
      }
      OperationRegion (REGS, SystemMemory, 0x2C40C100, 0x4)
      Field (REGS, DwordAcc, NoLock, Preserve)
      {
        REG0, 0x20
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 3)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 4)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 5)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 6)
        RST &= ~0x70
        REG0 = (REG0 & 0xFFFF) | 0xBB770000
        PWRV = 1
      }
      Method (_OFF)
      {
        RST |= 0x70
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 6)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 5)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 4)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 3)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { PWRR })
    Name (_PR3, Package () { PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS

    Device (RHUB)
    {
      Name (_ADR, Zero)

      Device (PRT1)
      {
        Name (_ADR, One)
        Name (_UPC, Package () { 0xFF, Zero, Zero, Zero })
        Name (_PLD, Package ()
        {
          Buffer ()
          {
            0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
      }

      Device (PRT2)
      {
        Name (_ADR, 2)
        Name (_UPC, Package () { 0xFF, Zero, Zero, Zero })
        Name (_PLD, Package ()
        {
          Buffer ()
          {
            0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
      }
    }
  }

  // USB3
  Device (USB3)
  {
    Name (_HID, "808622B7")
    Name (_CID, "PNP0D10")
    Name (_UID, One)
    Name (_CCA, One)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x2C500000, 0x100000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
      {
        269, 270, 271, 272, 273, 274, 275, 276, 278
      }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "dr_mode", "host" }
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      OperationRegion (RBUF, SystemMemory, 0x2C050000, 0x4)
      Field (RBUF, DwordAcc, NoLock, Preserve)
      {
        RST, 0x20
      }
      OperationRegion (REGS, SystemMemory, 0x2C50C100, 0x1C8)
      Field (REGS, DwordAcc, NoLock, Preserve)
      {
        REG0, 0x20,
        REG1, 0x20,
        Offset (0x1C0),
        REG2, 0x20,
        REG3, 0x20
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 7)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 8)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 9)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 10)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 11)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 12)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 13)
        RST &= ~0x1F00
        REG0 = (REG0 & 0xFFFF) | 0xBB770000
        REG0 = (REG0 & ~0x1) | 0x10
        REG1 &= ~0x1F00
        REG2 |= 0x10000000
        REG3 |= 0x10000000
        PWRV = 1
      }
      Method (_OFF)
      {
        RST |= 0x1F00
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 13)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 12)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 11)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 10)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 9)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 8)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 7)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { PWRR })
    Name (_PR3, Package () { PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS

    Device (RHUB)
    {
      Name (_ADR, Zero)

      Device (PRT1)
      {
        Name (_ADR, One)
        Name (_UPC, Package () { 0xFF, Zero, Zero, Zero })
        Name (_PLD, Package ()
        {
          Buffer ()
          {
            0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
      }

      Device (PRT2)
      {
        Name (_ADR, 2)
        Name (_UPC, Package () { 0xFF, Zero, Zero, Zero })
        Name (_PLD, Package ()
        {
          Buffer ()
          {
            0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
      }

      Device (PRT3)
      {
        Name (_ADR, 3)
        Name (_UPC, Package () { 0xFF, 0x03, Zero, Zero })
        Name (_PLD, Package ()
        {
          Buffer ()
          {
            0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
      }

      Device (PRT4)
      {
        Name (_ADR, 4)
        Name (_UPC, Package () { 0xFF, 0x03, Zero, Zero })
        Name (_PLD, Package ()
        {
          Buffer ()
          {
            0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x31, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
      }
    }
  }

  // USB PHY
  Device (UPHY)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 800)
    Name (_CCA, Zero)
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-usb-phy" }
      }
    })

    Device (PHY0)
    {
      Name (_ADR, Zero)
      Name (CTRL, Package () { ^^USB2 })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "reg", Zero },
          Package () { "enable", One }
        }
      })
    }

    Device (PHY1)
    {
      Name (_ADR, One)
      Name (CTRL, Package () { ^^USB3 })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "reg", One },
          Package () { "enable", One },
          Package () { "usb3", One }
        }
      })
    }

    Device (PHY2)
    {
      Name (_ADR, 2)
      Name (CTRL, Package () { ^^USB3 })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "reg", 2 },
          Package () { "enable", One }
        }
      })
    }
  }

  // SATA0
  Device (SAT0)
  {
    Name (_HID, "PRP0001")
    Name (_CLS, Package () { One, 0x06, One })
    Name (_UID, 0x2C600000)
    Name (_CCA, One)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x2C600000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 265 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-ahci" }
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      OperationRegion (RBUF, SystemMemory, 0x2C050000, 0x4)
      Field (RBUF, DwordAcc, NoLock, Preserve)
      {
        RST, 0x20
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 1)
        RST &= ~0x20000
        PWRV = 1
      }
      Method (_OFF)
      {
        RST |= 0x20000
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 1)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { \_SB.SATP, PWRR })
    Name (_PR3, Package () { \_SB.SATP, PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }

  // SATA1
  Device (SAT1)
  {
    Name (_HID, "PRP0001")
    Name (_CLS, Package () { One, 0x06, One })
    Name (_UID, 0x2C610000)
    Name (_CCA, One)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x2C610000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 266 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-ahci" }
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      OperationRegion (RBUF, SystemMemory, 0x2C050000, 0x4)
      Field (RBUF, DwordAcc, NoLock, Preserve)
      {
        RST, 0x20
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2C000000, 2)
        RST &= ~0x40000
        PWRV = 1
      }
      Method (_OFF)
      {
        RST |= 0x40000
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x2C000000, 2)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { \_SB.SATP, PWRR })
    Name (_PR3, Package () { \_SB.SATP, PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }

  // VDEC
  Device (VDEC) {
    Name (_HID, "PRP0001")
    Name (_UID, 0x24200000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x24200000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 529 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,d5500-vxd" }
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_PLL_ENABLE (0x24000000)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x24000000, 0)
        PWRV = 1
      }
      Method (_OFF)
      {
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x24000000, 0)
        ACPI_BAIKAL_CMU_PLL_DISABLE (0x24000000)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { PWRR })
    Name (_PR3, Package () { PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }

  // XGMAC0
  Device (XGM0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x30200000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x30200000, 0x10000)
      Memory32Fixed (ReadWrite, 0x30210000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
      {
        325, 326, 327, 328, 329, 330, 331, 332,
        333, 334, 335, 336, 337, 338, 339, 340,
        341
      }
    })
    Method (_DSD)
    {
/*
      If (CondRefOf (\_SB.XGM0.DSDO, Local1))
      {
        Local2 = SizeOf (Local1)
      }
      Else
      {
        Local2 = 0
      }
*/
      Local0 = Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package (/*4 + Local2*/) // manual update
        {
          Package () { "compatible", "baikal,bm1000-xgmac" },
          Package () { "reg-names", Package () {
            "stmmaceth",
            "xpcs"
          }},
          Package () { "interrupt-names", Package () {
            "macirq",  "dma_tx0", "dma_tx1","dma_tx2","dma_tx3",
                       "dma_tx4", "dma_tx5","dma_tx6","dma_tx7",
                       "dma_rx0", "dma_rx1","dma_rx2","dma_rx3",
                       "dma_rx4", "dma_rx5","dma_rx6","dma_rx7",
          } },
          Package () { "pcs-handle", \_SB.XMI0.XPC0 },
#ifdef USE_KRKX4
          Package () { "phy-mode", "xaui" },
          Package () { "managed", "auto" },
          Package () { "phy-handle", \_SB.MDIO.PR0C }
#endif
        }
      }
/*
      If (Local2 > 0)
      {
        For (Local3 = 0, Local3 < Local2, Local3++)
        {
          DerefOf (Local0[1])[4 + Local3] = Local1[Local3] // manual update
        }
      }
*/
      Return (Local0)
    }

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 2)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 3)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 4)
        PWRV = 1
      }
      Method (_OFF)
      {
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 4)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 3)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 2)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { \_SB.XGMP, PWRR })
    Name (_PR3, Package () { \_SB.XGMP, PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }

  // XGMAC0_MI
  Device (XMI0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x30210000)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0xF)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x30210000, 0x10000)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "snps,dw-xpcs-mi" },
        Package () { "reg-names", "indirect" },
        Package () { "reg-io-width", 4 },
      }
    })

    // XGMAC0_PCS
    Device(XPC0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x30218000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 355 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bt1-xpcs" },
        }
      })
    }
  }

  // XGMAC1
  Device (XGM1)
  {
    Name (_HID, "AMDI8003")
    Name (_UID, 0)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x30220000, 0x10000)
      Memory32Fixed (ReadWrite, 0x30230000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
      {
        342, 343, 344, 345, 346, 347, 348, 349,
        350, 351, 352, 353, 354, 355, 356, 357,
        358
      }
    })
    Method (_DSD)
    {
/*
      If (CondRefOf (\_SB.GMC1.DSDO, Local1))
      {
        Local2 = SizeOf (Local1)
      }
      Else
      {
        Local2 = 0
      }
*/
      Local0 = Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package (/*4 + Local2*/) // manual update
        {
          Package () { "reg-names", Package () {
            "stmmaceth",
            "xpcs"
          }},
          Package () { "interrupt-names", Package () {
            "macirq",  "dma_tx0", "dma_tx1","dma_tx2","dma_tx3",
                       "dma_tx4", "dma_tx5","dma_tx6","dma_tx7",
                       "dma_rx0", "dma_rx1","dma_rx2","dma_rx3",
                       "dma_rx4", "dma_rx5","dma_rx6","dma_rx7",
          } },
          Package () { "pcs-handle", \_SB.XMI1.XPC1 },
#ifdef USE_KRKX4
          Package () { "phy-mode", "xaui" },
          Package () { "managed", "auto" },
          Package () { "phy-handle", \_SB.MDIO.PR0E }
#endif
        }
      }
/*
      If (Local2 > 0)
      {
        For (Local3 = 0, Local3 < Local2, Local3++)
        {
          DerefOf (Local0[1])[4 + Local3] = Local1[Local3] // manual update
        }
      }
*/
      Return (Local0)
    }

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 5)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 6)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 7)
        PWRV = 1
      }
      Method (_OFF)
      {
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 7)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 6)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 5)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { \_SB.XGMP, PWRR })
    Name (_PR3, Package () { \_SB.XGMP, PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }

  // XGMAC1_MI
  Device (XMI1)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x30230000)
    Name (_CCA, Zero)
    Method (_STA)
    {
      Return (0x0)
    }
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x30230000, 0x10000)
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "snps,dw-xpcs-mi" },
        Package () { "reg-names", "indirect" },
        Package () { "reg-io-width", 4 },
      }
    })

    // XGMAC1_PCS
    Device(XPC1)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x30238000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 356 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bt1-xpcs" },
        }
      })
    }
  }

  // GMAC0
  Device (GMC0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x30240000)
    Name (_CCA, One)
    Method (_CRS)
    {
      Local0 = ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x30240000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 323 }
      }

      If (CondRefOf (\_SB.GMC0.GPIO, Local1))
      {
        Local0 = ConcatenateResTemplate (Local0, Local1)
      }

      Return (Local0)
    }
    Method (_DSD)
    {
      Local0 = Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bm1000-gmac" },
          Package () { "max-speed", 1000 },
          Package () { "reg", 3 },
          Package () { "phy-mode", "rgmii-id" },
          Package () { "stmmac-clk", 50000000 },
          Package () { "snps,fixed-burst", One },
          Package () { "snps,txpbl", 4 },
          Package () { "snps,rxpbl", 4 },
          Package () { "snps,blen", Package () { Zero, Zero, Zero, Zero, Zero, Zero, 4 } },
          Package () { "baikal,prop0", Package () { Zero } },
          Package () { "baikal,prop1", Package () { Zero } }
        }
      }

      If (CondRefOf (\_SB.GMC0.RSGP, Local1))
      {
        DerefOf (DerefOf (Local0[1])[9])[0] = "snps,reset-gpios"
        DerefOf (DerefOf (Local0[1])[9])[1] = DerefOf (Local1)
      }

      If (CondRefOf (\_SB.GMC0.RSDL, Local1))
      {
        DerefOf (DerefOf (Local0[1])[10])[0] = "snps,reset-delays-us"
        DerefOf (DerefOf (Local0[1])[10])[1] = DerefOf (Local1)
      }

      Return (Local0)
    }

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 8)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 9)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 10)
        PWRV = 1
      }
      Method (_OFF)
      {
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 10)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 9)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 8)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { \_SB.XGBP, PWRR })
    Name (_PR3, Package () { \_SB.XGBP, PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS

    Device (GPHY)
    {
      Name (_ADR, Zero)
    }
  }

  // GMAC1
  Device (GMC1)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x30250000)
    Name (_CCA, One)
    Method (_CRS)
    {
      Local0 = ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x30250000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 324 }
      }

      If (CondRefOf (\_SB.GMC1.GPIO, Local1))
      {
        Local0 = ConcatenateResTemplate (Local0, Local1)
      }

      Return (Local0)
    }
    Method (_DSD)
    {
      Local0 = Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bm1000-gmac" },
          Package () { "max-speed", 1000 },
          Package () { "reg", 3 },
          Package () { "phy-mode", "rgmii-id" },
          Package () { "stmmac-clk", 50000000 },
          Package () { "snps,fixed-burst", One },
          Package () { "snps,txpbl", 4 },
          Package () { "snps,rxpbl", 4 },
          Package () { "snps,blen", Package () { Zero, Zero, Zero, Zero, Zero, Zero, 4 } },
          Package () { "baikal,prop0", Package () { Zero } },
          Package () { "baikal,prop1", Package () { Zero } }
        }
      }

      If (CondRefOf (\_SB.GMC1.RSGP, Local1))
      {
        DerefOf (DerefOf (Local0[1])[9])[0] = "snps,reset-gpios"
        DerefOf (DerefOf (Local0[1])[9])[1] = DerefOf (Local1)
      }

      If (CondRefOf (\_SB.GMC1.RSDL, Local1))
      {
        DerefOf (DerefOf (Local0[1])[10])[0] = "snps,reset-delays-us"
        DerefOf (DerefOf (Local0[1])[10])[1] = DerefOf (Local1)
      }

      Return (Local0)
    }

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 11)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 12)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 13)
        PWRV = 1
      }
      Method (_OFF)
      {
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 13)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 12)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 11)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { \_SB.XGBP, PWRR })
    Name (_PR3, Package () { \_SB.XGBP, PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS

    Device (GPHY)
    {
      Name (_ADR, Zero)
    }
  }

  // GPU MALI
  Device (GPU0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x2A200000)
    Name (_CCA, One)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x2A200000, 0x4000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 207, 208, 206 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "arm,mali-t628" },
        Package () { "interrupt-names", Package () { "job", "mmu", "gpu" } },
        Package () { "operating-points", Package () {
          400000000, 10000000,
          450000000, 10000000,
          500000000, 10000000,
          550000000, 10000000,
          600000000, 10000000,
          650000000, 10000000,
          700000000, 10000000,
          750000000, 10000000
        }}
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      OperationRegion (RBUF, SystemMemory, 0x2A050000, 0x4)
      Field (RBUF, DwordAcc, NoLock, Preserve)
      {
        RST, 0x20
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_PLL_ENABLE (0x2A000000)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x2A000000, 0)
        RST &= ~0x3
        ID0 = 0xC2000800
        ID1 = 0
        ID2 = 0
        ID3 = 0
        ID4 = 0
        SMCB = (SMCF = SMCB)
        PWRV = 1
      }
      Method (_OFF)
      {
        RST |= 0x3
        ACPI_BAIKAL_CMU_CLKCH_DISABLE2 (0x2A000000, 0)
        ACPI_BAIKAL_CMU_PLL_DISABLE2 (0x2A000000)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { PWRR })
    Name (_PR3, Package () { PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }

  // HDMI
  Device (HDMI)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x30280000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x30280000, 0x20000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 363 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,hdmi" },
        Package () { "reg-io-width", 4 }
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      OperationRegion (RBUF, SystemMemory, 0x30050000, 0x4)
      Field (RBUF, DwordAcc, NoLock, Preserve)
      {
        RST, 0x20
      }
      OperationRegion (REGS, SystemMemory, 0x30280000, 0x20000)
      Field (REGS, DwordAcc, NoLock, Preserve)
      {
        Offset (0x410),
        HSTB, 0x20,
        HSTA, 0x20,
        Offset (0x600),
        HST0, 0x20,
        HST1, 0x20,
        HST2, 0x20,
        HST3, 0x20,
        HST4, 0x20,
        HST5, 0x20,
        HST6, 0x20,
        HST7, 0x20,
        HST8, 0x20,
        HST9, 0x20,
        Offset (0x7FC),
        HMUT, 0x20,
        Offset (0x201C),
        HIM0, 0x20,
        Offset (0x4348),
        HIM1, 0x20,
        Offset (0x4358),
        HIM2, 0x20,
        Offset (0x4368),
        HIM3, 0x20,
        Offset (0xC018),
        HIM4, 0x20,
        HPY0, 0x20,
        Offset (0xC09C),
        HIM5, 0x20,
        HIM6, 0x20,
        Offset (0xC408),
        HIM7, 0x20,
        Offset (0xCC08),
        HIM8, 0x20,
        Offset (0xD010),
        HIM9, 0x20,
        HIMA, 0x20,
        Offset (0x14020),
        HIMB, 0x20,
        Offset (0x1F814),
        HIMC, 0x20,
        HIMD, 0x20,
        HI20, 0x20,
        HI21, 0x20,
        HI22, 0x20,
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 16)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 17)
        RST &= ~0x1000
        HMUT = 0x3
        HIM0 = 0xFF
        HIM1 = 0xFF
        HIM2 = 0xFF
        HIM3 = 0xFF
        HIM4 = 0xFF
        HIM5 = 0xFF
        HIM6 = 0xFF
        HIM7 = 0xFF
        HIM8 = 0xFF
        HIM9 = 0xFF
        HIMA = 0xFF
        HIMB = 0xFF
        HIMC = 0xFF
        HIMD = 0xFF
        HST0 = 0xFF
        HST1 = 0xFF
        HST2 = 0xFF
        HST3 = 0xFF
        HST4 = 0xFF
        HST5 = 0xFF
        HST6 = 0xFF
        HST7 = 0xFF
        HST8 = 0xFF
        HST9 = 0xFF
        HMUT = 0
        HIM5 = 0x8
        HIM6 = 0x88
        HI22 = 0
        HI20 = 0
        HIMC = 0x8
        HIMD = 0x88
        HSTA = 0x3
        HST5 = 0x3
        HPY0 = 0xF2
        HSTB = 0x3D
        HIM4 = ~0xF2
        HSTB = 0x3D
        HST4 = ~0x3D
        PWRV = 1
      }
      Method (_OFF)
      {
        RST |= 0x1000
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 17)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 16)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { \_SB.XGBP, PWRR })
    Name (_PR3, Package () { \_SB.XGBP, PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }

  // VDU
  Device (VDU0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x202D0000)
    Name (_CCA, Zero)
    Method (_CRS)
    {
      Local0 = ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x30260000, 0x1000)
        Memory32Fixed (ReadWrite, 0x202D0000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 144, 361 }
      }

      If (CondRefOf (\_SB.VDU0.GPIO, Local1))
      {
        Local0 = ConcatenateResTemplate (Local0, Local1)
      }

      Return (Local0)
    }

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Name (CTX, 0)
      Method (_STA)
      {
        Return (PWRV)
      }
      OperationRegion (RBUF, SystemMemory, 0x20050008, 0x4)
      Field (RBUF, DwordAcc, NoLock, Preserve)
      {
        RST, 0x20
      }
      OperationRegion (REGS, SystemMemory, 0x30260000, 0x1000)
      Field (REGS, DwordAcc, NoLock, Preserve)
      {
        CR1, 0x20,
        Offset (0x8),
        HTR, 0x20,
        VTR1, 0x20,
        VTR2, 0x20,
        PCTR, 0x20,
        Offset (0x1c),
        IMR, 0x20,
        Offset (0x24),
        ISCR, 0x20,
        DBAR, 0x20,
        Offset (0xffc),
        MRR, 0x20,
      }
      Name (TBUF, Buffer (0x24) {})
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30000000, 15)
        ACPI_BAIKAL_CMU_PLL_ENABLE (0x30010000)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30010000, 0)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, 26)
        ACPI_BAIKAL_CMU_PLL_ENABLE (0x20010000)
        RST &= ~0x2100

        If (CTX) {
          CreateDwordField (TBUF, 0, REG0)
          CreateDwordField (TBUF, 0x10, REG4)
          CreateDwordField (TBUF, 0x14, REG5)
          CreateDwordField (TBUF, 0x18, REG6)
          CreateDwordField (TBUF, 0x20, REG8)

          MRR = REG8
          ISCR = REG6
          IMR = REG5
          PCTR = REG4
          CR1 = REG0 & ~0x1

          CTX = 0
        }

        PWRV = 1
      }
      Method (_OFF)
      {
        If (CSTA) {
          CreateDwordField (TBUF, 0, REG0)
          CreateDwordField (TBUF, 0x4, REG1)
          CreateDwordField (TBUF, 0x8, REG2)
          CreateDwordField (TBUF, 0xc, REG3)
          CreateDwordField (TBUF, 0x10, REG4)
          CreateDwordField (TBUF, 0x14, REG5)
          CreateDwordField (TBUF, 0x18, REG6)
          CreateDwordField (TBUF, 0x1c, REG7)
          CreateDwordField (TBUF, 0x20, REG8)

          REG0 = CR1
          REG1 = HTR
          REG2 = VTR1
          REG3 = VTR2
          REG4 = PCTR
          REG5 = IMR
          REG6 = ISCR
          REG7 = DBAR
          REG8 = MRR

          CTX = 1
        }
        Else
        {
          CTX = 0
        }
        RST |= 0x2100
        ACPI_BAIKAL_CMU_PLL_DISABLE (0x20010000)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, 26)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30010000, 0)
        ACPI_BAIKAL_CMU_PLL_DISABLE (0x30010000)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30000000, 15)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { \_SB.XGBP, PWRR })
    Name (_PR3, Package () { \_SB.XGBP, PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS

    Method (CISE, 1)
    {
      If (Arg0 == 0)
      {
        ACPI_BAIKAL_CMU_CLKCH_IS_ENABLED (0x30010000, 0)
      }
      Else
      {
        ACPI_BAIKAL_CMU_PLL_IS_ENABLED (0x20010000)
      }
      Return (ID0)
    }

    Method (CEN, 1)
    {
      If (Arg0 == 0)
      {
        ACPI_BAIKAL_CMU_PLL_ENABLE (0x30010000)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x30010000, 0)
      }
    }

    Method (CDIS, 1)
    {
      If (Arg0 == 0)
      {
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x30010000, 0)
        ACPI_BAIKAL_CMU_PLL_DISABLE (0x30010000)
      }
    }

    Method (CSET, 2)
    {
      If (Arg0 == 0)
      {
        ACPI_BAIKAL_CMU_PLL_SET_RATE (0x30010000, 27000000, Arg1)
        ACPI_BAIKAL_CMU_CLKCH_SET_RATE (0x30010000, 0, Arg1)
      }
      Else
      {
        ACPI_BAIKAL_CMU_PLL_SET_RATE (0x20010000, 27000000, Arg1)
      }
    }

    Method (CGET, 1)
    {
      If (Arg0 == 0)
      {
        ACPI_BAIKAL_CMU_CLKCH_GET_RATE (0x30010000, 0)
      }
      Else
      {
        ACPI_BAIKAL_CMU_PLL_GET_RATE (0x20010000, 27000000)
      }
      Return (ID0)
    }

    Method (_DSD)
    {
      Local0 = Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,vdu" },
          Package () { "interrupt-names", Package () { "lvds_irq", "hdmi_irq" } },
          Package () { "baikal,hdmi-bridge", \_SB.HDMI },
          Package () { "baikal,prop0", Zero },
          Package () { "baikal,prop1", Package () { Zero } },
          Package () { "baikal,prop2", Zero }
        }
      }

      If (CondRefOf (\_SB.VDU0.PNL0, Local1))
      {
        DerefOf (DerefOf (Local0[1])[3])[0] = "baikal,lvds-panel"
        DerefOf (DerefOf (Local0[1])[3])[1] = Local1
      }

      If (CondRefOf (\_SB.VDU0.ENGP, Local1))
      {
        DerefOf (DerefOf (Local0[1])[4])[0] = "enable-gpios"
        DerefOf (DerefOf (Local0[1])[4])[1] = DerefOf (Local1)
      }

      If (CondRefOf (\_SB.VDU0.LVLN, Local1))
      {
        DerefOf (DerefOf (Local0[1])[5])[0] = "lvds-lanes"
        DerefOf (DerefOf (Local0[1])[5])[1] = DerefOf (Local1)
      }

      Return (Local0)
    }
  }

  // I2S
  Device (I2S0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x20220000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x20220000, 0x10000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 136, 137, 138, 139 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-i2s" },
        Package () { "system-clock-frequency", 12000000 }
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    OperationRegion (RBUF, SystemMemory, 0x20050000, 0xC)
    Field (RBUF, DwordAcc, NoLock, Preserve)
    {
      RST1, 0x20,
      Offset (0x8),
      RST2, 0x20
    }
    Name (PSVL, 3)
    Method (_PSC)
    {
      Return (PSVL)
    }
    Method (_PS0)
    {
      RST1 &= ~0x8000
      RST2 &= ~0x80000000
      PSVL = 0
    }
    Method (_PS3)
    {
      RST2 |= 0x8000
      RST1 |= 0x80000000
      PSVL = 3
    }
  }

  // HDA
  Device (HDA0)
  {
    Name (_HID, "PRP0001")
    Name (_UID, 0x202C0000)
    Name (_CCA, Zero)
    Name (_CRS, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x202C0000, 0x1000)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 86 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "baikal,bm1000-hda" },
        Package () { "force-polling-mode", One },
        Package () { "broken-response-irq", One },
        Package () { "increment-codec-address", One },
        Package () { "cyclic-codec-probe", One }
      }
    })

    ACPI_BAIKAL_SMC_CMU_DATA
    PowerResource (PWRR, 0, 0)
    {
      Name (PWRV, 1)
      Method (_STA)
      {
        Return (PWRV)
      }
      OperationRegion (RBUF, SystemMemory, 0x20050008, 0x4)
      Field (RBUF, DwordAcc, NoLock, Preserve)
      {
        RST, 0x20
      }
      Method (_ON)
      {
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, 15)
        ACPI_BAIKAL_CMU_CLKCH_ENABLE (0x20000000, 16)
        RST &= ~0x4001
        PWRV = 1
      }
      Method (_OFF)
      {
        RST |= 0x4001
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, 16)
        ACPI_BAIKAL_CMU_CLKCH_DISABLE (0x20000000, 15)
        PWRV = 0
      }
    }
    Name (_PR0, Package () { PWRR })
    Name (_PR3, Package () { PWRR })
    ACPI_BAIKAL_PWM_PS_METHODS
  }
}
