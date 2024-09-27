/** @file
  Copyright (c) 2020 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifdef USE_I2S
Device (\_SB.I2C1.PR18)
{
  Name (_HID, "PRP0001")
  Name (_UID, 0x20250018)
  Method (_STA)
  {
    Return (0xF)
  }
  Name (_CRS, ResourceTemplate ()
  {
    I2CSerialBusV2 (0x18, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.I2C1")
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
      Package () { "reset-gpios", Package () { \_SB.I2C1.PR18, Zero, Zero, One } }
    }
  })
}

/* SND BAIKAL */
Device (\_SB.SND0)
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
      Package () { "baikal,cpu-dai", \_SB.I2S0 },
      Package () { "baikal,audio-codec", \_SB.I2C1.PR18 },
      Package () { "baikal,dai-name", "tlv320aic3x" },
      Package () { "baikal,codec-name", "tlv320aic3x-hifi" },
      Package () { "baikal,stream-name", "tlv320aic3x hifi" }
    }
  })
}

/* I2S */
Method (\_SB.I2S0._STA) { Return (0xF) }
#else
/* I2S */
Method (\_SB.I2S0._STA) { Return (Zero) }
#endif

/* SD/eMMC */
#if defined(USE_EMMC) || defined(USE_SD)
Method (\_SB.MMC0._STA) { Return (0xF) }
#else
Method (\_SB.MMC0._STA) { Return (Zero) }
#endif

/* HDA */
#ifdef USE_HDA
Method (\_SB.HDA0._STA) { Return (0xF) }
#else
Method (\_SB.HDA0._STA) { Return (Zero) }
#endif

#ifdef USE_LVDS
Name (\_SB.VDU0.GPIO, ResourceTemplate ()
{
  GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 16 }
})
Name (\_SB.VDU0.ENGP, Package ()
{
  \_SB.VDU0, Zero, Zero, One
})
Name (\_SB.VDU0.LVLN, 2)

Device (\_SB.VDU0.PNL0)
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

Device (\_SB.VDU0.BCKL)
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

Device (\_SB.VDU0.KEYS)
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
    GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 18, 17 }
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
        Package () { "gpios", Package () { \_SB.VDU0.KEYS, Zero, Zero, One } }
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
        Package () { "gpios", Package () { \_SB.VDU0.KEYS, Zero, One, One } }
      }
    })
  }
}
#endif
