/** @file
  Copyright (c) 2020 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiPlatform.h"

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

#define BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(ClusterId, Id, CtiId, PortId, EtmAddr, DbgAddr, CtiAddr)  \
          Device (ETM0)                                                                                 \
          {                                                                                             \
            Name (_HID, "ARMHC500")                                                                     \
            Name (_UID, Id)                                                                             \
            Name (_CCA, Zero)                                                                           \
            Method (_STA)                                                                               \
            {                                                                                           \
              Return (0xF)                                                                              \
            }                                                                                           \
            Name (_CRS, ResourceTemplate ()                                                             \
            {                                                                                           \
              Memory32Fixed (ReadWrite, EtmAddr, 0x1000)                                                \
            })                                                                                          \
            Name (_DSD, Package ()                                                                      \
            {                                                                                           \
              ToUUID ("ab02a46b-74c7-45a2-bd68-f7d344ef2153"),                                          \
              Package ()                                                                                \
              {                                                                                         \
                0,                                                                                      \
                1,                                                                                      \
                Package ()                                                                              \
                {                                                                                       \
                  1,                                                                                    \
                  ToUUID ("3ecbc8b6-1d0e-4fb3-8107-e627f805c6cd"),                                      \
                  1,                                                                                    \
                  Package () { 0, PortId, \_SB.PKG.CLU ## ClusterId.FUN0, 1 }                           \
                }                                                                                       \
              }                                                                                         \
            })                                                                                          \
          }                                                                                             \
                                                                                                        \
          Device (DBG0)                                                                                 \
          {                                                                                             \
            Name (_HID, "ARMHC503")                                                                     \
            Name (_UID, Id)                                                                             \
            Name (_CCA, Zero)                                                                           \
            Method (_STA)                                                                               \
            {                                                                                           \
              Return (0xF)                                                                              \
            }                                                                                           \
            Name (_CRS, ResourceTemplate ()                                                             \
            {                                                                                           \
              Memory32Fixed (ReadWrite, DbgAddr, 0x1000)                                                \
            })                                                                                          \
          }                                                                                             \
                                                                                                        \
          Device (CTI0)                                                                                 \
          {                                                                                             \
            Name (_HID, "ARMHC500")                                                                     \
            Name (_UID, CtiId)                                                                          \
            Name (_CCA, Zero)                                                                           \
            Method (_STA)                                                                               \
            {                                                                                           \
              Return (0xF)                                                                              \
            }                                                                                           \
            Name (_CRS, ResourceTemplate ()                                                             \
            {                                                                                           \
              Memory32Fixed (ReadWrite, CtiAddr, 0x1000)                                                \
            })                                                                                          \
            Name (_DSD, Package ()                                                                      \
            {                                                                                           \
              ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),                                          \
              Package ()                                                                                \
              {                                                                                         \
                Package () { "arm,coresight-cti-v8-arch", One },                                        \
                Package () { "arm,cs-dev-assoc", ^ETM0 }                                                \
              }                                                                                         \
            })                                                                                          \
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

DefinitionBlock ("Dsdt.aml", "DSDT", 2, "BAIKAL", "BKLEDSDT", 1)
{
  Scope (_SB_)
  {
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

    Device (CTI0)
    {
      Name (_HID, "ARMHC500")
      Name (_UID, 16)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x05004000, 0x1000)
      })

      Device (TRG0)
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
            Package () { "arm,trig-in-sigs", 0 },
            Package () { "arm,trig-conn-name", "ccn_dbgwatchtrig" }
          }
        })
      }

      Device (TRG1)
      {
        Name (_ADR, One)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "arm,trig-in-sigs", Package () { 2, 3, 4, 5 } },
            Package () { "arm,trig-in-types", Package () { 18, 19, 20, 17 } },
            Package () { "arm,trig-out-sigs", Package () { 6, 7 } },
            Package () { "arm,trig-out-types", Package () { 21, 21 } },
            Package () { "arm,cs-dev-assoc", \_SB.STM0 }
          }
        })
      }

      Device (TRG3)
      {
        Name (_ADR, 3)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "arm,trig-out-sigs", Package () { 0, 1 } },
            Package () { "arm,trig-out-types", Package () { 3, 4 } },
            Package () { "arm,trig-conn-name", "cctr" }
          }
        })
      }

      Device (TRG4)
      {
        Name (_ADR, 4)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "arm,trig-out-sigs", 2 },
            Package () { "arm,trig-conn-name", "ccn_pmusnapshot" }
          }
        })
      }

      Device (TRG5)
      {
        Name (_ADR, 5)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "arm,trig-out-sigs", Package () { 3, 4 } },
            Package () { "arm,trig-out-types", Package () { 1, 1 } },
            Package () { "arm,trig-conn-name", "gic_cs_ppii" }
          }
        })
      }
    }

    Device (CTI1)
    {
      Name (_HID, "ARMHC500")
      Name (_UID, 17)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x05005000, 0x1000)
      })

      Device (TRG0)
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
            Package () { "arm,trig-in-sigs", Package () { 0, 1 } },
            Package () { "arm,trig-in-types", Package () { 13, 12 } },
            Package () { "arm,trig-out-sigs", Package () { 0, 1 } },
            Package () { "arm,trig-out-types", Package () { 15, 16 } },
            Package () { "arm,cs-dev-assoc", \_SB.PKG.CLU0.ETF0 }
          }
        })
      }

      Device (TRG1)
      {
        Name (_ADR, One)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "arm,trig-in-sigs", Package () { 2, 3 } },
            Package () { "arm,trig-in-types", Package () { 13, 12 } },
            Package () { "arm,trig-out-sigs", Package () { 2, 3 } },
            Package () { "arm,trig-out-types", Package () { 15, 16 } },
            Package () { "arm,cs-dev-assoc", \_SB.PKG.CLU1.ETF0 }
          }
        })
      }

      Device (TRG2)
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
            Package () { "arm,trig-in-sigs", Package () { 4, 5 } },
            Package () { "arm,trig-in-types", Package () { 13, 12 } },
            Package () { "arm,trig-out-sigs", Package () { 4, 5 } },
            Package () { "arm,trig-out-types", Package () { 15, 16 } },
            Package () { "arm,cs-dev-assoc", \_SB.PKG.CLU2.ETF0 }
          }
        })
      }

      Device (TRG3)
      {
        Name (_ADR, 3)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "arm,trig-in-sigs", Package () { 6, 7 } },
            Package () { "arm,trig-in-types", Package () { 13, 12 } },
            Package () { "arm,trig-out-sigs", Package () { 6, 7 } },
            Package () { "arm,trig-out-types", Package () { 15, 16 } },
            Package () { "arm,cs-dev-assoc", \_SB.PKG.CLU3.ETF0 }
          }
        })
      }
    }

    Device (CTI2)
    {
      Name (_HID, "ARMHC500")
      Name (_UID, 18)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x05006000, 0x1000)
      })

      Device (TRG0)
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
            Package () { "arm,trig-in-sigs", Package () { 0, 1 } },
            Package () { "arm,trig-in-types", Package () { 13, 12 } },
            Package () { "arm,trig-out-sigs", Package () { 0, 1 } },
            Package () { "arm,trig-out-types", Package () { 15, 16 } },
            Package () { "arm,cs-dev-assoc", \_SB.ETR0 }
          }
        })
      }

      Device (TRG1)
      {
        Name (_ADR, One)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "arm,trig-in-sigs", Package () { 2, 3 } },
            Package () { "arm,trig-in-types", Package () { 13, 12 } },
            Package () { "arm,trig-out-sigs", Package () { 2, 3 } },
            Package () { "arm,trig-out-types", Package () { 15, 16 } },
            Package () { "arm,cs-dev-assoc", \_SB.ETB0 }
          }
        })
      }

      Device (TRG2)
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
            Package () { "arm,trig-in-sigs", Package () { 4, 5 } },
            Package () { "arm,trig-in-types", Package () { 13, 12 } },
            Package () { "arm,trig-out-sigs", Package () { 4, 5 } },
            Package () { "arm,trig-out-types", Package () { 15, 16 } },
            Package () { "arm,cs-dev-assoc", \_SB.ETF0 }
          }
        })
      }

      Device (TRG3)
      {
        Name (_ADR, 3)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "arm,trig-out-sigs", Package () { 6, 7 } },
            Package () { "arm,trig-out-types", Package () { 15, 16 } },
            Package () { "arm,cs-dev-assoc", \_SB.TPIU }
          }
        })
      }
    }

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

          BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(0, 0, 8, 0, 0x05240000, 0x05210000, 0x05220000)
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

          BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(0, 1, 9, 1, 0x05340000, 0x05310000, 0x05320000)
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

          BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(1, 2, 10, 0, 0x05440000, 0x05410000, 0x05420000)
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

          BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(1, 3, 11, 1, 0x05540000, 0x05510000, 0x05520000)
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

          BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(2, 4, 12, 0, 0x05640000, 0x05610000, 0x05620000)
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

          BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(2, 5, 13, 1, 0x05740000, 0x05710000, 0x05720000)
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

          BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(3, 6, 14, 0, 0x05840000, 0x05810000, 0x05820000)
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

          BAIKAL_ACPI_PROCESSOR_CORESIGHT_NODES(3, 7, 15, 1, 0x05940000, 0x05910000, 0x05920000)
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
      Name (_UID, 4)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      /* CMU id, clock name, frequency, is_osc27 */
      Name (PROP, Package ()
      {
        0x20000000, "bm1000-avlsp-cmu0", 1200000000, Zero
      })
      /* Device reference, clock name, clock id, con_id */
      Name (CLKS, Package ()
      {
        Zero, "gpio", 0, Zero,
        Zero, "uart1", 1, Zero,
        Zero, "uart2", 2, Zero,
        ^I2S0, "apb", 3, "i2sclk",
        ^SPI0, "spi", 4, Zero,
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
        ^ESPI, "espi", 5, Zero,
#else
        Zero, "espi", 5, Zero,
#endif
        ^I2C0, "i2c1", 6, Zero,
        ^I2C1, "i2c2", 7, Zero,
        ^TMR0, "timer1", 8, "timer",
        ^TMR1, "timer2", 9, "timer",
        ^TMR2, "timer3", 10, "timer",
        ^TMR3, "timer4", 11, "timer",
        ^DMA0, "hclk", 12, "hclk",
        ^SMB0, "smbus1", 13, Zero,
        ^SMB1, "smbus2", 14, Zero,
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
        ^HDA0, "hda_sys_clk", 15, "hda_sys_clk",
        ^HDA0, "hda_clk48", 16, "hda_clk48",
#else
        Zero, "hda_sys_clk", 15, Zero,
        Zero, "hda_clk48", 16, Zero,
#endif
        Zero, "mshc_axi", 17, Zero,
        ^MMC0, "mshc_ahb", 18, "bus",
        ^MMC0, "mshc_tx_x2", 19, "core",
        Zero, "mshc_b", 20, Zero,
        Zero, "mshc_tm", 21, Zero,
        Zero, "mshc_cqetm", 22, Zero,
        Zero, "hwa_clu", 23, Zero,
        Zero, "hwa_clu_hf", 24, Zero,
        Zero, "hwa_axi", 25, Zero,
        Zero, "vdu_axi", 26, Zero,
        Zero, "smmu", 27, Zero
      })
    }

    Device (CLK5)
    {
      Name (_HID, "BKLE0001")
      Name (_UID, 5)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      /* CMU id, clock name, frequency, is_osc27 */
      Name (PROP, Package ()
      {
        0x20010000, "bm1000-avlsp-cmu1", 1039500000, One
      })
      /* Device reference, clock name, fixed-factor clock, con_id */
      Name (CLKS, Package ()
      {
        ^VDU0, "lvds_clk", Package () { 2, 1, 7 }, "lvds_pclk"
      })
    }

    Device (CLK6)
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

    Device (CLK7)
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
        Zero, "sata_ref_alt_clk", 0, Zero,
        Zero, "sata_aclk_u0", 1, Zero,
        Zero, "sata_aclk_u1", 2, Zero,
        ^UPHY.PHY0, "usb2_phy0_ref_clk", 3, "phy0_clk",
        ^UPHY.PHY0, "usb2_phy1_ref_clk", 4, "phy1_clk",
        Zero, "usb2_aclk", 5, Zero,
        Zero, "usb2_clk_sofitp", 6, Zero,
        ^UPHY.PHY1, "usb3_phy0_ref_clk", 7, "phy0_clk",
        ^UPHY.PHY1, "usb3_phy1_ref_clk", 8, "phy1_clk",
        ^UPHY.PHY2, "usb3_phy2_ref_clk", 9, "phy0_clk",
        ^UPHY.PHY2, "usb3_phy3_ref_clk", 10, "phy1_clk",
        Zero, "usb3_aclk", 11, Zero,
        Zero, "usb3_clk_sofitp", 12, Zero,
        Zero, "usb3_clk_suspend", 13, Zero,
        Zero, "smmu_aclk", 14, Zero,
        ^DMA1, "dmac_aclk", 15, "apb_pclk",
        Zero, "gic_aclk", 16, Zero
      })
    }

    Device (CLK8)
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
        ^HDMI, "csr50mhz", 0, "iahb",
        ^GMC0, "gmac0_tx2", 10, "tx2_clk",
        ^GMC1, "gmac1_tx2", 13, "tx2_clk",
        Zero, "hdmi_aclk", 15, Zero,
        ^HDMI, "isfr", 17, "isfr"
      })
    }

    Device (CLK9)
    {
      Name (_HID, "BKLE0001")
      Name (_UID, 9)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      /* CMU id, clock name, frequency, is_osc27 */
      Name (PROP, Package ()
      {
        0x30010000, "bm1000-xgbe-cmu1", 25250000, One
      })
      /* Device reference, clock name, clock id, con_id */
      Name (CLKS, Package ()
      {
        ^VDU0, "pixelclk", 0, "hdmi_pclk"
      })
    }

    Device (TMR0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x20290000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
          Package () { "compatible", "baikal,bm1000-dw-apb-timer" }
        }
      })
    }

    Device (TMR1)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x20290014)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
          Package () { "compatible", "baikal,bm1000-dw-apb-timer" }
        }
      })
    }

    Device (TMR2)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x20290028)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
          Package () { "compatible", "baikal,bm1000-dw-apb-timer" }
        }
      })
    }

    Device (TMR3)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x2029003C)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
          Package () { "compatible", "baikal,bm1000-dw-apb-timer" }
        }
      })
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
      Method (_STA)
      {
        Return (0xF)
      }
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
      Method (_STA)
      {
        Return (0xF)
      }
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
      Method (_STA)
      {
        Return (0xF)
      }
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
      Method (_STA)
      {
        Return (0xF)
      }
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
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x20200000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 131 }
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
            Package () { "snps,nr-gpios", 32 },
#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
            Package () { "line-name", "pcie-x8-clock" },
            Package () { "gpio-hog", One },
            Package () { "gpios", Package () { One, One } },
            Package () { "output-high", One }
#endif
          }
        })
      }
    }

    // SPI
    Device (SPI0)
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
        Memory32Fixed (ReadWrite, 0x20210000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 132 }
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 24, 25, 26, 27 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "num-cs", 4 },
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
          Package () { "cs-gpios", Package ()
          {
            ^SPI0, Zero, Zero, One,
            ^SPI0, Zero, One, One,
            ^SPI0, Zero, 2, One,
            ^SPI0, Zero, 3, One
          }}
        }
      })

      Device (PR00)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x20210000)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          SPISerialBusV2 (Zero, PolarityHigh, FourWireMode, 8, ControllerInitiated, 12500000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.SPI0")
        })
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "baikal,partitions", Package ()
            {
              "bl1",       0x000000, 0x040000,
              "dtb",       0x040000, 0x040000,
              "uefi-vars", 0x080000, 0x0C0000,
              "fip",       0x140000, 0x6C0000
            }}
          }
        })
      }
#elif defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
          Package () { "cs-gpios", Package () { Zero } }
        }
      })
#else
        }
      })
#endif
    }

    // DMAC LSP
    Device (DMA0) {
      Name (_HID, "BKLE0005")
      Name (_UID, 0x202B0000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x2C630000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive)
        {
          255, 256, 257, 258, 259, 260, 261, 262, 263
        }
      })
    }

    // UART1
    Device (COM0)
    {
      Name (_HID, "HISI0031")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x20230000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 133 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "reg-shift", 2 },
          Package () { "reg-io-width", 4 },
          Package () { "clock-frequency", 7361963 }
        }
      })
    }

    // UART2
    Device (COM1)
    {
      Name (_HID, "HISI0031")
      Name (_UID, One)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x20240000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 134 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "reg-shift", 2 },
          Package () { "reg-io-width", 4 },
          Package () { "clock-frequency", 7361963 }
        }
      })
    }

    // I2C0
    Device (I2C0)
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
        Memory32Fixed (ReadWrite, 0x20250000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 140 }
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

#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
      Device (PR18)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x20250018)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          I2CSerialBusV2 (0x18, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
          GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 4 }
        })
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "compatible", "ti,tlv320aic3x" },
            Package () { "ai3x-micbias-vg", One },
            Package () { "ai3x-ocmv", One },
            Package () { "reset-gpios", Package () { ^PR18, Zero, Zero, One } }
          }
        })
      }
#endif

#if defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
      Device (PR08)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x20250008)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          I2CSerialBusV2 (0x08, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
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

      Device (PR1A)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x2025001A)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          I2CSerialBusV2 (0x1A, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
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

      Device (PR50)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x20250050)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          I2CSerialBusV2 (0x50, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
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

      Device (PR51)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x20250051)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          I2CSerialBusV2 (0x51, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
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

      Device (PR53)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x20250053)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          I2CSerialBusV2 (0x53, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
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
#elif defined(BAIKAL_DBM10)
      Device (PR56)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x20250056)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          I2CSerialBusV2 (0x56, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C0")
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
#endif
    }

    // I2C1
    Device (I2C1)
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
        Memory32Fixed (ReadWrite, 0x20260000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 141 }
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

#ifdef BAIKAL_DBM20
      Device (PR56)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x20260056)
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
#endif
    }

    // SMBUS1
    Device (SMB0)
    {
      Name (_HID, "PRP0001")
      Name (_CID, "PNP0500")
      Name (_UID, 0x20270000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x20270000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 142 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bm1000-smbus" },
          Package () { "clock-frequency", 100000 }
        }
      })
      Name (CLK, 50000000)
    }

    // SMBUS2
    Device (SMB1)
    {
      Name (_HID, "PRP0001")
      Name (_CID, "PNP0500")
      Name (_UID, 0x20280000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x20280000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 143 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bm1000-smbus" },
          Package () { "clock-frequency", 100000 }
        }
      })
      Name (CLK, 50000000)
    }

#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
    // ESPI
    Device (ESPI)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x202A0000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x202A0000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 135 }
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 0 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,bm1000-espi" },
          Package () { "cs-gpios", Package () { ^ESPI, Zero, Zero, One } }
        }
      })

      Device (PR00)
      {
        Name (_HID, "PRP0001")
        Name (_UID, 0x202A0001)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_CRS, ResourceTemplate ()
        {
          SPISerialBusV2 (Zero, PolarityHigh, FourWireMode, 8, ControllerInitiated, 10000000, ClockPolarityLow, ClockPhaseSecond, "\\_SB.ESPI")
        })
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "compatible", "jedec,spi-nor" },
            Package () { "reg", Zero }
          }
        })
      }
    }
#endif

    // SD/eMMC
    Device (MMC0)
    {
      Name (_HID, "BKLE0004")
      Name (_CID, "PNP0D40")
      Name (_UID, 0x202E0000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
          Package () { "no-mmc", 1 },
          Package () { "no-sdio", 1 },
          Package () { "disable-wp", 1 },
          Package () { "bus-width", 4 },
          Package () { "max-frequency", 25000000 }
        }
      })
    }

    // PVT3
    Device (PVT3)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x26200000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
      Method (_STA)
      {
        Return (0xF)
      }
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
      Method (_STA)
      {
        Return (0xF)
      }
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
      Method (_STA)
      {
        Return (0xF)
      }
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

      Device (RHUB)
      {
        Name (_ADR, Zero)
        Method (_STA)
        {
          Return (0xF)
        }

        Device (PRT1)
        {
          Name (_ADR, One)
          Method (_STA)
          {
            Return (0xF)
          }
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
          Method (_STA)
          {
            Return (0xF)
          }
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
      Method (_STA)
      {
        Return (0xF)
      }
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

      Device (RHUB)
      {
        Name (_ADR, Zero)
        Method (_STA)
        {
          Return (0xF)
        }

        Device (PRT1)
        {
          Name (_ADR, One)
          Method (_STA)
          {
            Return (0xF)
          }
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
          Method (_STA)
          {
            Return (0xF)
          }
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
          Method (_STA)
          {
            Return (0xF)
          }
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
          Method (_STA)
          {
            Return (0xF)
          }
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
      Method (_STA)
      {
        Return (0xF)
      }
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
        Method (_STA)
        {
          Return (0xF)
        }
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
        Method (_STA)
        {
          Return (0xF)
        }
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
        Method (_STA)
        {
          Return (0xF)
        }
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
      Method (_STA)
      {
        Return (0xF)
      }
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
    }

    // SATA1
    Device (SAT1)
    {
      Name (_HID, "PRP0001")
      Name (_CLS, Package () { One, 0x06, One })
      Name (_UID, 0x2C610000)
      Name (_CCA, One)
      Method (_STA)
      {
        Return (0xF)
      }
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
    }

    // VDEC
    Device (VDEC) {
      Name (_HID, "PRP0001")
      Name (_UID, 0x24200000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
    }

#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
    // Internal MDIO
    Device (MDIO)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 200)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 30, 29 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,mdio-gpio" },
          Package () { "bus-id", Zero },
          Package () { "mdc-gpio", Package ()
          {
            ^MDIO, Zero, Zero, Zero
          }},
          Package () { "mdio-gpio", Package ()
          {
            ^MDIO, Zero, One, Zero
          }}
        }
      })

      Device (PR0C)
      {
        Name (_ADR, 0x0C)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "reg", 0x0C },
            Package () { "mv,line-mode", "KR" },
            Package () { "mv,host-mode", "KX4" }
          }
        })
      }

      Device (PR0E)
      {
        Name (_ADR, 0x0E)
        Method (_STA)
        {
          Return (0xF)
        }
        Name (_DSD, Package ()
        {
          ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package ()
          {
            Package () { "reg", 0x0E },
            Package () { "mv,line-mode", "KR" },
            Package () { "mv,host-mode", "KX4" }
          }
        })
      }
    }

    // XGMAC0
    Device (XGM0)
    {
      Name (_HID, "AMDI8001")
      Name (_UID, Zero)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "amd,dma-freq", 156250000 },
          Package () { "amd,ptp-freq", 156250000 },
          Package () { "amd,per-channel-interrupt", One },
          Package () { "amd,speed-set", Zero },
          Package () { "phy-mode", "xgmii" },
          Package () { "mac-address", Package () { 0x4C, 0xA5, 0x15, 0x00, 0x00, 0x00 } },
          Package () { "be,pcs-mode", "KX4" },
          Package () { "ext-phy-handle", ^MDIO.PR0C }
        }
      })
    }

    // XGMAC1
    Device (XGM1)
    {
      Name (_HID, "AMDI8001")
      Name (_UID, One)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "amd,dma-freq", 156250000 },
          Package () { "amd,ptp-freq", 156250000 },
          Package () { "amd,per-channel-interrupt", One },
          Package () { "amd,speed-set", Zero },
          Package () { "phy-mode", "xgmii" },
          Package () { "mac-address", Package () { 0x4C, 0xA5, 0x15, 0x00, 0x00, 0x01 } },
          Package () { "be,pcs-mode", "KX4" },
          Package () { "ext-phy-handle", ^MDIO.PR0E }
        }
      })
    }

    Device (SFP0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 1000)
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
          Package () { "compatible", "sff,sfp" },
          Package () { "i2c-bus", ^I2C0 }
        }
      })
    }

    Device (SFP1)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 1001)
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
          Package () { "compatible", "sff,sfp" },
          Package () { "i2c-bus", ^I2C0 }
        }
      })
    }
#endif

    // GMAC0
    Device (GMC0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x30240000)
      Name (_CCA, One)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x30240000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 323 }
#ifdef BAIKAL_MBM20
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 19 }
#endif
      })
      Name (_DSD, Package ()
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
#ifdef BAIKAL_MBM20
          Package () { "snps,reset-gpios", Package ()
          {
            ^GMC0, Zero, Zero, One,
          }},
          Package () { "snps,reset-delays-us", Package () { Zero, 10000, 50000 } }
#endif
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
      Name (_UID, 0x30250000)
      Name (_CCA, One)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x30250000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 324 }
#ifdef BAIKAL_MBM20
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 20 }
#endif
      })
      Name (_DSD, Package ()
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
#ifdef BAIKAL_MBM20
          Package () { "snps,reset-gpios", Package ()
          {
            ^GMC1, Zero, Zero, One,
          }},
          Package () { "snps,reset-delays-us", Package () { Zero, 10000, 50000 } }
#endif
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

    // GPU MALI
    Device (GPU0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x2A200000)
      Name (_CCA, One)
      Method (_STA)
      {
        Return (0xF)
      }
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
    }

    // VDU
    Device (VDU0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x202D0000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x30260000, 0x1000)
        Memory32Fixed (ReadWrite, 0x202D0000, 0x1000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 144, 361 }
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20) || defined(BAIKAL_MBM20)
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 16 }
#endif
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,vdu" },
          Package () { "interrupt-names", Package () { "lvds_irq", "hdmi_irq" } },
          Package () { "baikal,hdmi-bridge", ^HDMI },
#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20) || defined(BAIKAL_MBM20)
          Package () { "baikal,lvds-panel", PNL0 },
          Package () { "enable-gpios", Package () { ^VDU0, Zero, Zero, One } },
          Package () { "lvds-lanes", 2 }
        }
      })

      Device (PNL0)
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

      Device (BCKL)
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

      Device (GPIO)
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
              Package () { "gpios", Package () { ^^GPIO, Zero, Zero, One } }
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
              Package () { "gpios", Package () { ^^GPIO, Zero, One, One } }
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
              Package () { "gpios", Package () { ^^GPIO, Zero, 2, One } }
            }
          })
        }
      }
#else
        }
      })
#endif
    }

    // HDMI
    Device (HDMI)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x30280000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
    }

    // I2S
    Device (I2S0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x20220000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
      Name (_CRS, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x20220000, 0x10000)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 136, /* 137, */ 138, 139 }
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "snps,designware-i2s" },
          Package () { "system-clock-frequency", 12000000 }
        }
      })
    }

#if defined(BAIKAL_DBM10) || defined(BAIKAL_DBM20)
    // SND BAIKAL
    Device (SND0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 900)
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
          Package () { "compatible", "baikal,snd-soc-baikal" },
          Package () { "baikal,cpu-dai", ^I2S0 },
          Package () { "baikal,audio-codec", ^I2C0.PR18 },
          Package () { "baikal,dai-name", "tlv320aic3x" },
          Package () { "baikal,codec-name", "tlv320aic3x-hifi" },
          Package () { "baikal,stream-name", "tlv320aic3x hifi" }
        }
      })
    }

    // HDA
    Device (HDA0)
    {
      Name (_HID, "PRP0001")
      Name (_UID, 0x202C0000)
      Name (_CCA, Zero)
      Method (_STA)
      {
        Return (0xF)
      }
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
    }
#elif defined(BAIKAL_MBM10) || defined(BAIKAL_MBM20)
    // SND BAIKAL SIMPLE
    Device (SND0)
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
#ifdef BAIKAL_MBM10
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 26, 27 }
#else
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 26, 29 }
#endif
      })
      Name (_DSD, Package ()
      {
        ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package ()
        {
          Package () { "compatible", "baikal,simple-audio-card" },
          Package () { "mic-det-gpio", Package () { ^SND0, Zero, Zero, One } },
          Package () { "hp-det-gpio", Package () { ^SND0, Zero, One, Zero } },
          Package () { "baikal,cpu-dai", ^I2S0 },
          Package () { "baikal,audio-codec", ^I2C0.PR1A },
          Package () { "baikal,codec-name", "nau8822-hifi" }
        }
      })
    }

    Device (LEDS)
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
            Package () { "gpios", Package () { ^LED0, Zero, Zero, Zero } }
          }
        })
      }
    }
#endif
  }
}
