/** @file
  Copyright (c) 2020 - 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Platform/Pcie.h>

#include "AcpiPlatform.h"

DefinitionBlock (__FILE__, "SSDT", 2, "BAIKAL", "SSDTPCI0", 1) {
  External (\_SB.GPIO, DeviceObj)
  External (\_SB.GPIO.GPIP, DeviceObj)
  External (\_SB.STA0, FieldUnitObj)
  External (\_SB.STA1, FieldUnitObj)
  External (\_SB.STA2, FieldUnitObj)

  Scope (_SB_) {
    // PCIe LCRU
    Device (CRU0) {
      Name (_ADR, 0x02000000)
      Name (_UID, 0x02000000)
      Name (_CRS, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0x02000000, 0x80000)
      })
    }

    // PCIe0 (x4 #0)
    Device (PCI0) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE0_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE0_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA, 0, Serialized) {
        if ((\_SB.STA0 & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                    BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
          Return (0x0)
        }

        Return (0xF)
      }

      Name (_CRS, ResourceTemplate () {
        QWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite, Zero, 0x40000000, 0x7FFFFFFF, 0x3C0000000, 0x40000000)
        QWordIo (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange, Zero, Zero, 0x000FFFFF, 0x70000000, 0x00100000, ,,, TypeTranslation)
        WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode, Zero, Zero, 255, Zero, 256)
      })

      Name (NUML, 4)
      Name (NUMV, 4)
      Name (LCRU, Package () { ^CRU0, 0 })

      Device (RES0) {
        Name (_HID, "PNP0C02")
        Name (_UID, 100)
        Name (_CRS, ResourceTemplate () {
          Memory32Fixed (ReadWrite, 0x40100000, 0x10000000)
          Memory32Fixed (ReadWrite, 0x02200000, 0x00001000)
          Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 458, 461 }
#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
          GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 6 }
#endif
        })
      }

      NATIVE_PCIE_OSC
    }

#ifdef BAIKAL_DBM
    // PCIe1 (x4 #1)
    Device (PCI1) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE1_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE1_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA, 0, Serialized) {
        if ((\_SB.STA1 & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                    BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
          Return (0x0)
        }

        Return (0xF)
      }

      Name (_CRS, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0x50100000, 0x10000000)
        Memory32Fixed (ReadWrite, 0x02210000, 0x00001000)
        QWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite, Zero, 0x40000000, 0x7FFFFFFF, 0x4C0000000, 0x40000000)
        QWordIo (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange, Zero, 0x00100000, 0x001FFFFF, 0x70000000, 0x00100000, ,,, TypeTranslation)
        WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode, Zero, Zero, 255, Zero, 256)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 434, 437 }
      })

      Name (NUML, 4)
      Name (NUMV, 4)
      Name (LCRU, Package () { ^CRU0, One })

      Device (RES0) {
        Name (_HID, "PNP0C02")
        Name (_UID, 200)
        Name (_CRS, ResourceTemplate () {
          Memory32Fixed (ReadWrite, 0x50100000, 0x10000000)
          Memory32Fixed (ReadWrite, 0x02210000, 0x00001000)
          Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 434, 437 }
        })
      }

      NATIVE_PCIE_OSC
    }
#endif

    // PCIe2 (x8)
    Device (PCI2) {
      Name (_HID, EISAID ("PNP0A08"))
      Name (_CID, EISAID ("PNP0A03"))
      Name (_UID, BAIKAL_ACPI_PCIE2_SEGMENT)
      Name (_CCA, One)
      Name (_SEG, BAIKAL_ACPI_PCIE2_SEGMENT)
      Name (_BBN, Zero)

      Method (_STA, 0, Serialized) {
        if ((\_SB.STA2 & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                    BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
          Return (0x0)
        }

        Return (0xF)
      }

      Name (_CRS, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0x60100000, 0x10000000)
        Memory32Fixed (ReadWrite, 0x02220000, 0x00001000)
        QWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite, Zero, 0x80000000, 0xFFFFFFFF, 0x580000000, 0x80000000)
        QWordIo (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange, Zero, 0x00200000, 0x002FFFFF, 0x70000000, 0x00100000, ,,, TypeTranslation)
        WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode, Zero, Zero, 255, Zero, 256)
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 410, 413 }
#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
        GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 3 }
#endif
      })

      Name (NUML, 8)
      Name (NUMV, 4)
      Name (LCRU, Package () { ^CRU0, 2 })

      Device (RES0) {
        Name (_HID, "PNP0C02")
        Name (_UID, 300)
        Name (_CRS, ResourceTemplate () {
          Memory32Fixed (ReadWrite, 0x60100000, 0x10000000)
          Memory32Fixed (ReadWrite, 0x02220000, 0x00001000)
          Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) { 410, 413 }
#if defined (BAIKAL_MBM10) || defined (BAIKAL_MBM20)
          GpioIo (Exclusive, PullDefault, , , IoRestrictionNone, "\\_SB.GPIO") { 3 }
#endif
        })
      }

      NATIVE_PCIE_OSC
    }
  }
}
