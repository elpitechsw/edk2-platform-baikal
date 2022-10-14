/** @file
  Copyright (c) 2021 - 2022, Andrei Warkentin <andreiw@mm.st><BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <BM1000.h>

#include "AcpiPlatform.h"

  Scope (_SB_) {
  Device (XXXX) {
    Name (_HID, EISAID ("PNP0A08"))
    Name (_CID, EISAID ("PNP0A03"))
    Name (_CCA, One)
    Name (_BBN, 0)

    //
    // The following NAMEOPs need to be patched to "individualize" this segment.
    //
    // _UID - e.g. BAIKAL_ACPI_PCIE0_SEGMENT
    // RUID - e.g. BM1000_PCIE0_IDX
    // _SEG - e.g. BAIKAL_ACPI_PCIE0_SEGMENT
    // CFGB - e.g. BM1000_PCIE0_CFG_BASE
    // MB32 - e.g. BM1000_PCIE0_MEM_BASE
    // MS32 - e.g. BM1000_PCIE0_MEM_SIZE
    // MBPF - e.g. the prefetchable window base
    // MSPF - e.g. the prefetchable window size
    // IOBA - e.g. BM1000_PCIE0_IO_MIN
    // IOCA - e.g. BM1000_PCIE0_IO_BASE
    // IOSI - e.g. BM1000_PCIE0_IO_SIZE
    // BUSC - max buses
    //
    // The default values are important so the NameOps are sized right
    // for patching in GeneratePcieSsdt/FindAndPatchNameOp.
    //
    // And uh, don't use 0xFFFFFFFFFFFFFFFF, as AML optimizes that as OnesOp.
    //
    Name (_UID, 0xAB)
    Name (_SEG, 0xAB)
    Name (RUID, 0xAB)
    Name (CFGB, 0xABCDEF0123456789)
    Name (MB32, 0xABCDEF0123456789)
    Name (MS32, 0xABCDEF0123456789)
    Name (MBPF, 0xABCDEF0123456789)
    Name (MSPF, 0xABCDEF0123456789)
    Name (IOBA, 0xABCDEF0123456789)
    Name (IOCA, 0xABCDEF0123456789)
    Name (IOSI, 0xABCDEF0123456789)
    Name (BUSC, 0xAB)

    Method (_STA, 0, Serialized) {
      OperationRegion (LCRU, SystemMemory, BM1000_PCIE_GPR_STATUS_REG(RUID), 0x4)
      Field (LCRU, DWordAcc, NoLock, Preserve) {
        Offset (0x00),
        STAR, 32
      }

      OperationRegion (CFGS, SystemMemory, CFGB, 0x10)
      Field (CFGS, ByteAcc, NoLock, Preserve) {
        Offset (0xE),
        TYPE, 8
      }

      if ((STAR & BM1000_PCIE_GPR_STATUS_LTSSM_STATE_MASK) !=
                  BM1000_PCIE_GPR_STATUS_LTSSM_STATE_L0) {
        Return (0x0)
      }

#ifdef PCIE_SINGLE_DEVICE_BUS_RANGE
      //
      // If we're only exposing one device, then don't bother
      // exposing bridges as we'll never be able to access the EPs
      // hanging off of them.
      //
      if ((TYPE & 0x7F) != 0) {
        Return (0x0)
      }
#endif // PCIE_SINGLE_DEVICE_BUS_RANGE
      Return (0xF)
    }

    Method (_CRS, 0, Serialized) {
      Name (RBUF, ResourceTemplate () {
        PROD_MEM_BUF(01)
        PROD_MEM_BUF(02)
        PROD_IO_BUF(03)
        PROD_BUS_BUF(04)
      })

      QRES_BUF_SET(01, MB32, MS32, 0)
      QRES_BUF_SET(02, MBPF, MSPF, 0)
      QRES_BUF_SET(03, IOBA, IOSI, IOCA - IOBA)
      WRES_BUF_SET(04, 0, BUSC, 0)

      Return (RBUF)
    }

#ifdef PCIE_NATIVE
    NATIVE_PCIE_OSC
#endif // PCIE_NATIVE

#ifdef PCIE_WITH_ECAM_RANGE
      Device(ECAM) {
        Name(_HID, EISAID("PNP0C02"))
        Method (_CRS, 0, Serialized) {
          Name (RBUF, ResourceTemplate () {
            CONS_MEM_BUF(01)
          })

          QRES_BUF_SET(01, CFGB, 0xFF00000, 0)
          Return (RBUF)
        }
      }
#endif // PCIE_WITH_ECAM_RANGE
  }
}
