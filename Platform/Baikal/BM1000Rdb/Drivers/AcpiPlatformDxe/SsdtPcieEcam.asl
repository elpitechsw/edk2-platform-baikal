/** @file
  Copyright (c) 2021 - 2022, Andrei Warkentin <andreiw@mm.st><BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#define PCIE_WITH_ECAM_RANGE
#define PCIE_SINGLE_DEVICE_BUS_RANGE

DefinitionBlock (__FILE__, "SSDT", 2, "BAIKAL", "ECAM", 1) {
  #include "SsdtPcieCommon.asl"
}
