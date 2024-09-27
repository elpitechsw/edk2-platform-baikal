/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/* ESPI devices */
Device (\_SB.ESP0.PR00)
{
  Name (_HID, "SPT0001")
  Name (_UID, PLATFORM_ADDR_OUT_CHIP(0, 0x00C40000))
  Method (_STA)
  {
    Return (0xF)
  }
  Name (_CRS, ResourceTemplate ()
  {
    SPISerialBusV2 (Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 10000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.ESP0")
  })
}

Device (\_SB.ESP0.PR01)
{
  Name (_HID, "SPT0001")
  Name (_UID, PLATFORM_ADDR_OUT_CHIP(0, 0x00C40001))
  Method (_STA)
  {
    Return (0xF)
  }
  Name (_CRS, ResourceTemplate ()
  {
    SPISerialBusV2 (One, PolarityLow, FourWireMode, 8, ControllerInitiated, 10000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.ESP0")
  })
}

Device (\_SB.ESP0.PR02)
{
  Name (_HID, "SPT0001")
  Name (_UID, PLATFORM_ADDR_OUT_CHIP(0, 0x00C40002))
  Method (_STA)
  {
    Return (0xF)
  }
  Name (_CRS, ResourceTemplate ()
  {
    SPISerialBusV2 (0x2, PolarityLow, FourWireMode, 8, ControllerInitiated, 10000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.ESP0")
  })
}

Device (\_SB.ESP0.PR03)
{
  Name (_HID, "SPT0001")
  Name (_UID, PLATFORM_ADDR_OUT_CHIP(0, 0x00C40003))
  Method (_STA)
  {
    Return (0xF)
  }
  Name (_CRS, ResourceTemplate ()
  {
    SPISerialBusV2 (0x3, PolarityLow, FourWireMode, 8, ControllerInitiated, 10000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.ESP0")
  })
}
