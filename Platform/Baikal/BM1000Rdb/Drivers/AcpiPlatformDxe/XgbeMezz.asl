/** @file
  Copyright (c) 2020 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifdef USE_KRKX4
Device (\_SB.MDIO)
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
    GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 28, 29, 30 }
  })
  Name (_DSD, Package ()
  {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
    Package ()
    {
      Package () { "compatible", "baikal,mdio-gpio" },
      Package () { "bus-id", Zero },
      Package () { "clock-frequency", 1000000 },
      Package () { "mdc-gpio", Package ()
      {
        \_SB.MDIO, Zero, Zero, Zero
      }},
      Package () { "mdio-gpio", Package ()
      {
        \_SB.MDIO, Zero, One, Zero
      }},
      Package () { "rst-gpio", Package ()
      {
        \_SB.MDIO, Zero, 2, One
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
    Name (_CRS, ResourceTemplate()
    {
        GpioInt (Level, ActiveLow, Shared, PullUp, 0, "\\_SB.GPIO") { 31 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "ethernet-phy-ieee802.3-c45" },
        Package () { "reg", 0x0C },
        Package () { "gpio-controller", One }, /* 12 pins */
        Package () { "reserved", 0x1CF },
        Package () { "gpio-unreserved", 0x1CF },
        Package () { "sfp", \_SB.SFP0 }
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
    Name (_CRS, ResourceTemplate()
    {
        GpioInt (Level, ActiveLow, Shared, PullDown, 0, "\\_SB.GPIO") { 31 }
    })
    Name (_DSD, Package ()
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package ()
      {
        Package () { "compatible", "ethernet-phy-ieee802.3-c45" },
        Package () { "reg", 0x0E },
        Package () { "gpio-controller", One }, /* 12 pins */
        Package () { "gpio-unreserved", 0x1CF },
        Package () { "sfp", \_SB.SFP1 }
      }
    })
  }
}

Device (\_SB.SFP0)
{
  Name (_HID, "PRP0001")
  Name (_UID, 1000)
  Name (_CCA, Zero)
  Method (_STA)
  {
    Return (0xF)
  }
  Name (_CRS, ResourceTemplate ()
  {
    GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.MDIO.PR0C") { 0, 1,2, 8 }
  })
  Name (_DSD, Package ()
  {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
    Package ()
    {
      Package () { "compatible", "sff,sfp" },
      Package () { "i2c-bus", \_SB.MDIO.PR0C },
      Package () { "mod-def0-gpio", Package ()
      {
        ^SFP0, Zero, 0, 1
      }},
      Package () { "tx-fault-gpio", Package ()
      {
        ^SFP0, Zero, 1, 0
      }},
      Package () { "los-gpio", Package ()
      {
        ^SFP0, Zero, 2, 0
      }},
      Package () { "tx-disable-gpio", Package ()
      {
        ^SFP0, Zero, 3, 0
      }}
    }
  })
}

Device (\_SB.SFP1)
{
  Name (_HID, "PRP0001")
  Name (_UID, 1001)
  Name (_CCA, Zero)
  Method (_STA)
  {
    Return (0xF)
  }
  Name (_CRS, ResourceTemplate ()
  {
    GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.MDIO.PR0E") { 0, 1,2, 8 }
  })
  Name (_DSD, Package ()
  {
    ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
    Package ()
    {
      Package () { "compatible", "sff,sfp" },
      Package () { "i2c-bus", \_SB.MDIO.PR0E },
      Package () { "mod-def0-gpio", Package ()
      {
        ^SFP1, Zero, 0, 1
      }},
      Package () { "tx-fault-gpio", Package ()
      {
        ^SFP1, Zero, 1, 0
      }},
      Package () { "los-gpio", Package ()
      {
        ^SFP1, Zero, 2, 0
      }},
      Package () { "tx-disable-gpio", Package ()
      {
        ^SFP1, Zero, 3, 0
      }}
    }
  })
}

/* XGMAC0 */
Method (\_SB.XGM0._STA) { Return (0xF) }
Name (\_SB.XGM0.DSDO, Package () {
  Package () { "phy-mode", "xaui" },
  Package () { "managed", "auto" },
  Package () { "phy-handle", \_SB.MDIO.PR0C }
})

/* XGMAC1 */
Method (\_SB.XGM1._STA) { Return (0xF) }
Name (\_SB.XGM1.DSDO, Package () {
  Package () { "phy-mode", "xaui" },
  Package () { "managed", "auto" },
  Package () { "phy-handle", \_SB.MDIO.PR0E }
})
#endif
