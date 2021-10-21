/** @file
  Copyright (c) 2021, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef BAIKAL_ACPI_H_
#define BAIKAL_ACPI_H_

#define BAIKAL_ACPI_HEADER(Signature, Type, Revision, Id)  { \
  /* UINT32  Signature       */                              \
  Signature,                                                 \
  /* UINT32  Length          */                              \
  sizeof (Type),                                             \
  /* UINT8   Revision        */                              \
  Revision,                                                  \
  /* UINT8   Checksum        */                              \
  0,                                                         \
  /* UINT8   OemId[6]        */                              \
  { 'B', 'A', 'I', 'K', 'A', 'L' },                          \
  /* UINT64  OemTableId      */                              \
  ((UINT64)(Id) << 32) | 0x454C4B42,                         \
  /* UINT32  OemRevision     */                              \
  1,                                                         \
  /* UINT32  CreatorId       */                              \
  0x454C4B42,                                                \
  /* UINT32  CreatorRevision */                              \
  1                                                          \
}

#define BAIKAL_ACPI_PCIE_X4_0_SEGMENT  0
#ifdef BAIKAL_DBM
#define BAIKAL_ACPI_PCIE_X4_1_SEGMENT  1
#define BAIKAL_ACPI_PCIE_X8_SEGMENT    2
#define BAIKAL_ACPI_PCIE_COUNT         3
#else
#define BAIKAL_ACPI_PCIE_X8_SEGMENT    1
#define BAIKAL_ACPI_PCIE_COUNT         2
#endif

#endif // BAIKAL_ACPI_H_
