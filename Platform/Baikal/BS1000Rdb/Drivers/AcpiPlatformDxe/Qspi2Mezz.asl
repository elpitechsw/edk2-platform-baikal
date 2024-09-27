/** @file
  Copyright (c) 2021 - 2024, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/* QSPI_2 devices */
Device (\_SB.SP01.PR00)
{
  Name (_HID, "SPT0001")
  Name (_UID, PLATFORM_ADDR_OUT_CHIP(0, 0x00C30000))
  Method (_STA)
  {
    Return (0xF)
  }
  Name (_CRS, ResourceTemplate ()
  {
    SPISerialBusV2 (Zero, PolarityLow, FourWireMode, 8, ControllerInitiated, 1000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.SP01")
  })
}

Device (\_SB.SP01.PR01)
{
  Name (_HID, "SPT0001")
  Name (_UID, PLATFORM_ADDR_OUT_CHIP(0, 0x00C30001))
  Method (_STA)
  {
    Return (0xF)
  }
  Name (_CRS, ResourceTemplate ()
  {
    SPISerialBusV2 (One, PolarityLow, FourWireMode, 8, ControllerInitiated, 1000000, ClockPolarityLow, ClockPhaseFirst, "\\_SB.SP01")
  })
}
