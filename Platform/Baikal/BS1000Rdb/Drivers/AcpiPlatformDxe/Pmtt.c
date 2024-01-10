/** @file
  Copyright (c) 2022 - 2023, Baikal Electronics, JSC. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi.h>
#include <Library/BaseMemoryLib.h>
#include "AcpiPlatform.h"

#include <BS1000.h>

#pragma pack(1)

typedef struct {
  EFI_ACPI_6_4_PMTT_COMMON_MEMORY_DEVICE  SocketHeader;
  UINT16                                  SocketIdentifier;
  UINT16                                  Reserved;
  EFI_ACPI_6_4_PMTT_COMMON_MEMORY_DEVICE  DimmHeader;
  UINT32                                  SmbiosHandle;
} BAIKAL_ACPI_PMTT_SOCKET_DIMM;

typedef struct {
  EFI_ACPI_6_4_PLATFORM_MEMORY_TOPOLOGY_TABLE  Header;
  BAIKAL_ACPI_PMTT_SOCKET_DIMM                 Device[PLATFORM_CHIP_COUNT * BS1000_DIMM_COUNT];
} BAIKAL_ACPI_PMTT;

#pragma pack()

STATIC BAIKAL_ACPI_PMTT_SOCKET_DIMM  SocketDimmTemplate = {
  /* EFI_ACPI_6_4_PMTT_COMMON_MEMORY_DEVICE  SocketHeader     */
  {
    /* UINT8   Type      */
    0,
    /* UINT8   Reserved  */
    EFI_ACPI_RESERVED_BYTE,
    /* UINT16  Length    */
    16,
    /* UINT16  Flags     */
    BIT0,
    /* UINT16  Reserved1 */
    EFI_ACPI_RESERVED_WORD,
    /* UINT32  NumberOfMemoryDevices */
    1
  },
  /* UINT16                                  SocketIdentifier */
  0,
  /* UINT16                                  Reserved         */
  EFI_ACPI_RESERVED_WORD,
  /* EFI_ACPI_6_4_PMTT_COMMON_MEMORY_DEVICE  DimmHeader       */
  {
    /* UINT8   Type      */
    2,
    /* UINT8   Reserved  */
    EFI_ACPI_RESERVED_BYTE,
    /* UINT16  Length    */
    16,
    /* UINT16  Flags     */
    BIT1,
    /* UINT16  Reserved1 */
    EFI_ACPI_RESERVED_WORD,
    /* UINT32  NumberOfMemoryDevices */
    1
  },
  /* UINT32                                  SmbiosHandle     */
  0
};

STATIC BAIKAL_ACPI_PMTT  Pmtt = {
  {
    /* EFI_ACPI_DESCRIPTION_HEADER  Header                */
    BAIKAL_ACPI_HEADER (
      EFI_ACPI_6_4_PLATFORM_MEMORY_TOPOLOGY_TABLE_SIGNATURE,
      BAIKAL_ACPI_PMTT,
      EFI_ACPI_6_4_MEMORY_TOPOLOGY_TABLE_REVISION,
      0x54544D50
      ),
    /* UINT32                       NumberOfMemoryDevices */
    BS1000_DIMM_COUNT,
  },
};

EFI_STATUS
PmttInit (
  EFI_ACPI_DESCRIPTION_HEADER  **Table
  )
{
  UINTN  ChipIdx;
  UINTN  Idx;
  UINTN  Num;

  for (ChipIdx = 0, Num = 0; ChipIdx < PLATFORM_CHIP_COUNT; ++ChipIdx) {
    for (Idx = 0; Idx < BS1000_DIMM_COUNT; ++Idx, ++Num) {
      CopyMem (&Pmtt.Device[Num], &SocketDimmTemplate, sizeof (SocketDimmTemplate));
      Pmtt.Device[Num].SocketIdentifier = Num;
      Pmtt.Device[Num].SmbiosHandle = (17 << 8 | Num) & 0x0000FFFF;
    }
  }

  *Table = (EFI_ACPI_DESCRIPTION_HEADER *) &Pmtt;
  return EFI_SUCCESS;
}
